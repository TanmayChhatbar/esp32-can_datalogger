import asyncio
from bleak import BleakScanner
from tpmss import *

TARGET_IDS = [tpms.id for tpms in TPMS_IDS]

def on_advertisement(device, adv):
    id = device.address
    if all([t.lower() != id.lower() for t in TARGET_IDS]):
        return

    tpms = next((t for t in TPMS_IDS if t.id.lower() == id.lower()), None)
    tpms.setData(adv)
    tpms.printData()
    
    # check with syringe
    v1 = 100                # [mL] initial syringe volume
    p1 = 14.7               # [psi] ambient pressure
    p2 = tpms.pressure      # [psi] measured pressure
    v2 = v1 * p1 / p2       # [mL] final syringe volume estimate
    print(f"\ts:\t{v2:.1f}")
    
async def main():
    print(f"Scanning for {TARGET_IDS} for 30 seconds...")
    async with BleakScanner(detection_callback=on_advertisement):
        await asyncio.sleep(30)

asyncio.run(main())
