import json
import requests

API_TOKEN="MZd27P3S1YaQhWhkpaZqGjB7qMiuUimdWcgJ179wv6wUcF1X73hTDKNiRNcI"

def lambda_handler(event, context):
    
    allMachines = getRequest("/machines/get/all", API_TOKEN).json()
    mostRecentMachineID = allMachines[len(allMachines)-1]['id']
    
    return {
        'statusCode': 200,
        'id': json.dumps(mostRecentMachineID)
    }

def getRequest(url: str, apitoken: str) -> str:
    HEADERS = {"User-Agent": "blood-tracker-api"}
    BASE = "https://www.hackthebox.eu/api"
    return requests.get(f"{BASE}{url}?api_token={apitoken}", headers=HEADERS)
    


