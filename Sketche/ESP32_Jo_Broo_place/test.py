import socket
import json

ESP32_IP = "192.168.178.26"
ESP32_PORT = 80

# Erstelle das JSON-Datenobjekt
led_data = {
    "led": "1",
    "r": "0",
    "g": "255",
    "b": "0"
}
json_data = json.dumps(led_data)

try:
    # Erstelle eine TCP-Verbindung zum ESP32
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((ESP32_IP, ESP32_PORT))

    # Sende Daten an den ESP32
    client_socket.sendall(json_data.encode())
    print("Daten erfolgreich gesendet:", json_data)

except Exception as e:
    print("Fehler beim Senden der Daten:", e)

finally:
    # Schließe die Verbindung, unabhängig davon, ob ein Fehler aufgetreten ist oder nicht
    if 'client_socket' in locals():
        client_socket.close()
