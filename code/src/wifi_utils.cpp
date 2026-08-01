#include "wifi_utils.h"

WebServer server(80);
const char *host = "esp32sd";
File uploadFile;

static const char INDEX_HTML_START[] PROGMEM = R"rawliteral(
<!doctype html>
<meta charset="utf-8">
<style>
    body {
        min-height: 100vh;
        display: grid;
        place-items: center;
        background: #ddd;
        font: 16px Arial
    }
    main {
        width: 680px;
        max-width: 92%;
        padding: 28px;
        background: #ffffff;
        border-radius: 20px;
        box-shadow: 0 0 20px rgba(0,0,0,.1)
    }
    table {
        width: 100%;
    }
    th,td {
        padding: 13px 12px;
        text-align: left;
        border-bottom: 1px solid #e2e8f0
    }

</style>
<main>
    <h1>ESP32-CAN Datalogger</h1>
)rawliteral";

const char DOWNLOAD_BUTTON[] PROGMEM = R"rawliteral(
    <button onclick="downloadFiles()">Download all log files</button>
)rawliteral";

const char INDEX_TABLE_START[] PROGMEM = R"rawliteral(
    <br>
    <br>
    <table>
        <thead><tr><th>Filename<th>Size
        <tbody>
)rawliteral";

const char INDEX_TABLE_END[] PROGMEM = R"rawliteral(
    </table>
)rawliteral";

const char INDEX_HTML_END[] PROGMEM = R"rawliteral(
</main>
)rawliteral";

const char HTML_DL_SCRIPT_START[] PROGMEM = R"rawliteral(
    <script>
        function downloadFiles() {
            const files = [
)rawliteral";
const char HTML_DL_SCRIPT_END[] PROGMEM = R"rawliteral(
            ];

            files.forEach((file, index) => {
                setTimeout(() => {
                    const link = document.createElement("a");
                    link.href = file;
                    link.download = "";
                    document.body.appendChild(link);
                    link.click();
                    link.remove();
                }, index * 500);
            });
        }
    </script>
)rawliteral";

void returnOK()
{
    server.send(200, "text/plain", "");
}

void returnFail(String msg)
{
    server.send(500, "text/plain", msg + "\r\n");
}

String getStorageDetails()
{
    uint64_t totalBytes = SD.cardSize();
    uint64_t usedBytes = SD.usedBytes();
    uint64_t freeBytes = totalBytes - usedBytes;

    float totalMB = totalBytes / (1024.0f * 1024.0f);
    float usedMB = usedBytes / (1024.0f * 1024.0f);
    float freeMB = freeBytes / (1024.0f * 1024.0f);

    float usedPercent = totalBytes > 0
                            ? (usedBytes * 100.0f) / totalBytes
                            : 0.0f;

    return "SD: " + String(usedMB, 1) + " MB used of " +
           String(totalMB, 1) + " MB (" +
           String(usedPercent, 1) + "%), " +
           String(freeMB, 1) + " MB free";
}

bool serveDir(File dir)
{
    String dataType = "text/plain";
    String page = INDEX_HTML_START;
    String script = HTML_DL_SCRIPT_START;
    // page += "<p>" + storage_details + "</p>";
    page += getStorageDetails();
    page += "<br><br>";
    page += DOWNLOAD_BUTTON;
    page += INDEX_TABLE_START;
    // add link to parent directory
    if (strcmp(dir.name(), "/") != 0 && strcmp(dir.name(), "") != 0)
    {
        page += "<tr><td><a href=\"..\">..</a> <td>-</tr>";
    }

    // add list of log files in SD card
    while (true)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            break;
        }
        if (strcmp(entry.name(), "System Volume Information") == 0)
        {
            entry.close();
            continue;
        }

        String name = entry.name();
        String url = entry.path();
        String size;
        bool isDir = entry.isDirectory();
        if (isDir)
        {
            size = "-";
            name += "/";
        }
        else
        {
            if (entry.size() > 1024 * 1024)
            {
                size = String(entry.size() / (1024 * 1024));
                size += " MB";
            }
            else if (entry.size() > 1024)
            {
                size = String(entry.size() / 1024);
                size += " KB";
            }
            else
            {
                size = String(entry.size());
                size += " B";
            }

            if (name.endsWith(".log"))
            {
                script += "                \"" + url + "\",\n";
            }
        }
        String output_table = "<tr><td><a href=\"";
        output_table += url;
        if (isDir)
        {
            output_table += "\">";
        }
        else
        {
            output_table += "\" download>";
        }
        output_table += name;
        output_table += "</a> <td>";
        output_table += size;
        output_table += "</tr>";
        page += output_table;
        entry.close();
    }
    page += INDEX_TABLE_END;

    page += INDEX_HTML_END;
    page += script;
    page += HTML_DL_SCRIPT_END;
    server.send(200, "text/html", page);
    return true;
}

void handleNotFound()
{
    File dir = SD.open(server.uri());
    if (dir.isDirectory())
    {
        serveDir(dir);
        return;
    }
    else // is file
    {
        String dataType = "text/plain";
        File dataFile = SD.open(server.uri());
        if (dataFile)
        {
            if (server.hasArg("download"))
            {
                String dataType = "application/octet-stream";
            }

            if (server.streamFile(dataFile, dataType) != dataFile.size())
            {
                Serial.println("Sent less data than expected!");
            }

            dataFile.close();
            return;
        }
    }
    String message = "SDCARD Not Detected\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (int i = 0; i < server.args(); i++)
    {
        message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    Serial.print(message);
}

bool wifiEnable()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // Wait for connection
    uint8_t i = 0;
    while (WiFi.status() != WL_CONNECTED && i++ < 20)
    { // wait 10 seconds
        delay(500);
    }
    if (i == 21)
    {
        Serial.print("Could not connect to ");
        Serial.println(ssid);
        return false;
    }
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(host))
    {
        MDNS.addService("http", "tcp", 80);
        Serial.println("MDNS responder started");
        Serial.print("You can now connect to http://");
        Serial.print(host);
        Serial.println(".local");
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    return true;
}

bool wifiDisable()
{
    server.close();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return true;
}
