import requests, json
from requests.packages.urllib3.exceptions import InsecureRequestWarning

requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

url2 = "https://gigachat.devices.sberbank.ru/api/v1/chat/completions"

with open("arc/system_prompt.txt", "r", encoding="utf-8") as file:
    system_data = file.read()


with open("src/buffer_code.txt", "r", encoding="utf-8") as file:
    code = file.read()

with open("src/access.json", "r", encoding="utf-8") as file:
    access_token = json.load(file)["access_token"]
    

payload = json.dumps({
  "model": "GigaChat-Pro",
  "messages": [
    {
      "role": "system",
      "content": system_data
    },
    {
      "role": "user",
      "content": code
    }
  ]
})

headers = {
    'Content-Type': 'application/json',
    'Accept': 'application/json',
    'Authorization': f'Bearer {access_token}'
}

response = requests.request("POST", url2, headers=headers, data=payload, verify=False)

with open("src/answer.json", "w", encoding="utf-8") as file:
    answer = {
        "answer": response.json()["choices"][0]["message"]["content"]
    }

    json.dump(answer, file, ensure_ascii=False)
