import requests, json
from requests.packages.urllib3.exceptions import InsecureRequestWarning

requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

url = "https://ngw.devices.sberbank.ru:9443/api/v2/oauth"
auth_key = "NmQ1Zjc3N2EtYmNmYi00Yzk4LTg4OGYtNWRkNWIwYmJiOTI4OjMzN2VkMzFmLWMzNjEtNGY0ZS1hNDZjLTYxMWNmZTU5NGU3OQ=="

payload={
  'scope': 'GIGACHAT_API_PERS'
}

headers = {
  'RqUID': 'cbe6b9c0-2231-4cd6-a62a-02678c4467d7',
  'Content-Type': 'application/x-www-form-urlencoded',
  'Accept': 'application/json',
  'Authorization': f'Basic {auth_key}'
}

response = requests.request("POST", url, headers=headers, data=payload, verify=False)

response = response.json()

with open("src/access.json", "w", encoding="utf-8") as file:
    json.dump(response, file, ensure_ascii=False)
    file.close()
