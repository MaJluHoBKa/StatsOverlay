import hashlib
import uuid
import json
import os
import platform
import subprocess
from typing import Optional

class HWIDActivator:
    SECRET_SEED = "zse4xdr5"

    @staticmethod
    def get_activation_file_path() -> str:
        """Возвращает путь к файлу активации в папке Documents/StatsOverlay"""
        documents_dir = os.path.join(os.path.expanduser("~"), "Documents")  # Путь к папке Documents
        activation_dir = os.path.join(documents_dir, "StatsOverlay")  # Папка StatsOverlay
        os.makedirs(activation_dir, exist_ok=True)  # Создаём папку, если её нет
        return os.path.join(activation_dir, "activation.json")  # Путь к файлу активации

    @staticmethod
    def get_hwid() -> str:
        """Генерирует HWID на основе стабильных характеристик системы"""
        hwid_components = {
            'machine_id': HWIDActivator._get_machine_id(),
            'processor_id': HWIDActivator._get_processor_id(),
            'disk_serial': HWIDActivator._get_disk_serial(),
        }
        hwid_str = json.dumps(hwid_components, sort_keys=True)
        return hashlib.sha256(hwid_str.encode()).hexdigest()

    @staticmethod
    def generate_key() -> str:
        """Генерирует ключ активации на основе HWID"""
        hwid = HWIDActivator.get_hwid()
        combined = f"{HWIDActivator.SECRET_SEED}:{hwid}"
        return hashlib.sha256(combined.encode()).hexdigest()[:16].upper()

    @staticmethod
    def activate(key: str) -> bool:
        """Активирует программу с указанным ключом"""
        if key == HWIDActivator.generate_key():
            activation_data = {
                'hwid': HWIDActivator.get_hwid(),
                'key': key,
                'activated': True
            }
            try:
                activation_file = HWIDActivator.get_activation_file_path()
                with open(activation_file, 'w') as f:
                    json.dump(activation_data, f)
                return True
            except Exception as e:
                print(f"Ошибка при сохранении активации: {e}")
                return False
        return False

    @staticmethod
    def check_activation() -> bool:
        """Проверяет, активирована ли программа"""
        activation_file = HWIDActivator.get_activation_file_path()
        if not os.path.exists(activation_file):
            return False

        try:
            with open(activation_file, 'r') as f:
                data = json.load(f)

            current_hwid = HWIDActivator.get_hwid()
            if data.get('hwid') != current_hwid:
                return False

            return data.get('key') == HWIDActivator.generate_key() and data.get('activated', False)
        except Exception as e:
            print(f"Ошибка при проверке активации: {e}")
            return False

    @staticmethod
    def _get_machine_id() -> str:
        """Получает machine ID системы (стабильный идентификатор)"""
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, 
                                r"SOFTWARE\Microsoft\Cryptography") as key:
                return winreg.QueryValueEx(key, "MachineGuid")[0]
        except Exception:
            return "default_machine_id"

    @staticmethod
    def _get_processor_id() -> str:
        """Получает ID процессора (стабильный идентификатор)"""
        try:
            result = subprocess.check_output(
                'wmic cpu get ProcessorId', 
                shell=True,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL
            )
            return result.decode().split('\n')[1].strip()
        except Exception:
            return "default_processor_id"

    @staticmethod
    def _get_disk_serial() -> str:
        """Получает серийный номер системного диска (стабильный идентификатор)"""
        try:
            result = subprocess.check_output(
                'wmic diskdrive where "index=0" get serialnumber',
                shell=True,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL
            )
            return result.decode().split('\n')[1].strip()
        except Exception:
            return "default_disk_serial"