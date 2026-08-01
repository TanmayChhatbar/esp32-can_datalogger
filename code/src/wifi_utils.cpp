#include "wifi_utils.h"

WebServer server(80);
const char *host = "esp32sd";
File uploadFile;

void returnOK()
{
    server.send(200, "text/plain", "");
}

void returnFail(String msg)
{
    server.send(500, "text/plain", msg + "\r\n");
}

bool loadFromSdCard(String path)
{
    String dataType = "text/plain";
    if (path.endsWith("/"))
    {
        path += "index.htm";
    }

    if (path.endsWith(".src"))
    {
        path = path.substring(0, path.lastIndexOf("."));
    }
    else if (path.endsWith(".htm"))
    {
        dataType = "text/html";
    }
    else if (path.endsWith(".css"))
    {
        dataType = "text/css";
    }
    else if (path.endsWith(".js"))
    {
        dataType = "application/javascript";
    }
    else if (path.endsWith(".png"))
    {
        dataType = "image/png";
    }
    else if (path.endsWith(".gif"))
    {
        dataType = "image/gif";
    }
    else if (path.endsWith(".jpg"))
    {
        dataType = "image/jpeg";
    }
    else if (path.endsWith(".ico"))
    {
        dataType = "image/x-icon";
    }
    else if (path.endsWith(".xml"))
    {
        dataType = "text/xml";
    }
    else if (path.endsWith(".pdf"))
    {
        dataType = "application/pdf";
    }
    else if (path.endsWith(".zip"))
    {
        dataType = "application/zip";
    }

    File dataFile = SD.open(path.c_str());
    if (dataFile.isDirectory())
    {
        path += "/index.htm";
        dataType = "text/html";
        dataFile = SD.open(path.c_str());
    }

    if (!dataFile)
    {
        return false;
    }

    if (server.hasArg("download"))
    {
        dataType = "application/octet-stream";
    }

    if (server.streamFile(dataFile, dataType) != dataFile.size())
    {
        Serial.println("Sent less data than expected!");
    }

    dataFile.close();
    return true;
}

void handleFileUpload()
{
    if (server.uri() != "/edit")
    {
        return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        if (SD.exists((char *)upload.filename.c_str()))
        {
            SD.remove((char *)upload.filename.c_str());
        }
        uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
        Serial.print("Upload: START, filename: ");
        Serial.println(upload.filename);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (uploadFile)
        {
            uploadFile.write(upload.buf, upload.currentSize);
        }
        Serial.print("Upload: WRITE, Bytes: ");
        Serial.println(upload.currentSize);
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (uploadFile)
        {
            uploadFile.close();
        }
        Serial.print("Upload: END, Size: ");
        Serial.println(upload.totalSize);
    }
}

void deleteRecursive(String path)
{
    File file = SD.open((char *)path.c_str());
    if (!file.isDirectory())
    {
        file.close();
        SD.remove((char *)path.c_str());
        return;
    }

    file.rewindDirectory();
    while (true)
    {
        File entry = file.openNextFile();
        if (!entry)
        {
            break;
        }
        String entryPath = path + "/" + entry.name();
        if (entry.isDirectory())
        {
            entry.close();
            deleteRecursive(entryPath);
        }
        else
        {
            entry.close();
            SD.remove((char *)entryPath.c_str());
        }
        yield();
    }

    SD.rmdir((char *)path.c_str());
    file.close();
}

void handleDelete()
{
    if (server.args() == 0)
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || !SD.exists((char *)path.c_str()))
    {
        returnFail("BAD PATH");
        return;
    }
    deleteRecursive(path);
    returnOK();
}

void handleCreate()
{
    if (server.args() == 0)
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg(0);
    if (path == "/" || SD.exists((char *)path.c_str()))
    {
        returnFail("BAD PATH");
        return;
    }

    if (path.indexOf('.') > 0)
    {
        File file = SD.open((char *)path.c_str(), FILE_WRITE);
        if (file)
        {
            file.write(0);
            file.close();
        }
    }
    else
    {
        SD.mkdir((char *)path.c_str());
    }
    returnOK();
}

void printDirectory()
{
    if (!server.hasArg("dir"))
    {
        return returnFail("BAD ARGS");
    }
    String path = server.arg("dir");
    if (path != "/" && !SD.exists((char *)path.c_str()))
    {
        return returnFail("BAD PATH");
    }
    File dir = SD.open((char *)path.c_str());
    path = String();
    if (!dir.isDirectory())
    {
        dir.close();
        return returnFail("NOT DIR");
    }
    dir.rewindDirectory();
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/json", "");

    server.sendContent("[");
    for (int cnt = 0; true; ++cnt)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            break;
        }

        String output;
        if (cnt > 0)
        {
            output = ',';
        }

        output += "{\"type\":\"";
        output += (entry.isDirectory()) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += entry.path();
        output += "\"";
        output += "}";
        server.sendContent(output);
        entry.close();
    }
    server.sendContent("]");
    dir.close();
}

void handleNotFound()
{
    if (loadFromSdCard(server.uri()))
    {
        return;
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
        Serial.print("Could not connect to");
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

    server.on("/list", HTTP_GET, printDirectory);
    server.on("/edit", HTTP_DELETE, handleDelete);
    server.on("/edit", HTTP_PUT, handleCreate);
    server.on(
        "/edit", HTTP_POST,
        []()
        {
            returnOK();
        },
        handleFileUpload);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    return true;
}
