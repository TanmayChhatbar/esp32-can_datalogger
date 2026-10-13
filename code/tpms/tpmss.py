class tpms:
    def __init__(self, id, name):
        self.id = id
        self.name = name
        self.raw_data = None
        self.temperature = None
        self.pressure = None
        self.company_id = None
        self.rssi = None
        self.service_uuids = None
        
    def setData(self, adv):
        for company_id, d in adv.manufacturer_data.items():
            self.raw_data = d
            self.company_id = company_id
        
        self.rssi = adv.rssi
        self.service_uuids = adv.service_uuids
        
        self.decodeData()
        
    def decodeData(self):
        self.pressure = (self.raw_data[1] * 256 + self.raw_data[2]) / 10.0 # [psi]
        self.temperature = self.raw_data[0] # [degC]
        
    def printData(self):
        print(f"TPMS {self.name}\t({self.id}) ({self.rssi}dBm)")
        # print(f"\tSvUUIDs:\t{self.service_uuids}")
        print(f"\tT:\t\t{self.temperature}")
        print(f"\tP:\t\t{self.pressure}")
        
    def printRawData(self):
        print(f"TPMS {self.name}\t({self.id}) ({self.rssi}dBm)")

TPMS_IDS = [
    tpms("37:39:0E:10:CD:5E", "Front"),
    tpms("37:39:0E:10:B8:A7", "Rear"),
    ]
