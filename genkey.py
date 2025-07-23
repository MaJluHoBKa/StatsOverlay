import hashlib
import json
import platform
import subprocess
import uuid
import os

def get_hwid():
    """Генерирует HWID на основе стабильных характеристик системы"""
    hwid_components = {
        'machine_id': _get_machine_id(),
        'processor_id': _get_processor_id(),
        'disk_serial': _get_disk_serial(),
    }
    hwid_str = json.dumps(hwid_components, sort_keys=True)
    return hashlib.sha256(hwid_str.encode()).hexdigest()

def generate_key(hwid):
    """Генерирует ключ активации на основе HWID и секретного слова"""
    secret_seed = 'unsigned'
    combined = f"{secret_seed}:{hwid}"
    return hashlib.sha256(combined.encode()).hexdigest()[:16].upper()

def _get_machine_id():
    """Получает machine ID системы"""
    try:
        if platform.system() == 'Linux':
            with open('/etc/machine-id') as f:
                return f.read().strip()
        elif platform.system() == 'Windows':
            import winreg
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, 
                              r"SOFTWARE\Microsoft\Cryptography") as key:
                return winreg.QueryValueEx(key, "MachineGuid")[0]
        return str(uuid.getnode())
    except Exception:
        return "default_machine_id"

def _get_processor_id():
    """Получает ID процессора"""
    try:
        if platform.system() == 'Windows':
            result = subprocess.check_output(
                'wmic cpu get ProcessorId', 
                shell=True,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL
            )
            return result.decode().split('\n')[1].strip()
        elif platform.system() == 'Linux':
            with open('/proc/cpuinfo') as f:
                for line in f:
                    if line.startswith('serial'):
                        return line.split(':')[1].strip()
                return "linux_cpu"
        return platform.processor()
    except Exception:
        return "default_processor_id"

def _get_disk_serial():
    """Получает серийный номер системного диска"""
    try:
        if platform.system() == 'Windows':
            result = subprocess.check_output(
                'wmic diskdrive where "index=0" get serialnumber',
                shell=True,
                stderr=subprocess.DEVNULL,
                stdin=subprocess.DEVNULL
            )
            return result.decode().split('\n')[1].strip()
        elif platform.system() == 'Linux':
            result = subprocess.check_output(
                ['lsblk', '-o', 'SERIAL', '-n', '-d'],
                stderr=subprocess.DEVNULL
            )
            return result.decode().strip()
        return "default_disk_serial"
    except Exception:
        return "default_disk_serial"

if __name__ == "__main__":
    print("Генератор ключей активации")
    print("--------------------------")
    
    # Вариант 1: Автоматически получить HWID текущей системы
    hwid = get_hwid()
    print(f"\nHWID текущей системы:\n{hwid}")
    
    # Вариант 2: Ввести HWID вручную
    user_input = input("\nВведите HWID для генерации ключа (или нажмите Enter для использования текущего): ").strip()
    
    if user_input:
        hwid = user_input
    
    activation_key = generate_key(hwid)
    
    print("\nРезультат:")
    print(f"HWID: {hwid}")
    print(f"Ключ активации: {activation_key}")
    
    input("\nНажмите Enter для выхода...")