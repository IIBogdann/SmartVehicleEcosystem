package com.example.smarttrafficsigns.ble

import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothProfile
import android.bluetooth.BluetoothGattDescriptor
import android.content.Context
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.util.UUID

/**
 * Clasa pentru gestionarea conexiunilor BLE și comunicarea cu dispozitivele ESP32-C3
 */
class BleConnection(private val context: Context) {

    companion object {
        private const val TAG = "BleConnection"
        
        // UUID-urile pentru serviciile și caracteristicile GATT
        // Aceste UUID-uri sunt sincronizate cu cele definite în firmware-ul ESP32-C3 (BleManager.h)
        val SERVICE_UUID = UUID.fromString("8f0e0d0c-0b0a-0908-0706-050403020100")
        val COMMAND_CHARACTERISTIC_UUID = UUID.fromString("8f0e0d0c-0b0a-0908-0706-050403020101")
        val STATUS_CHARACTERISTIC_UUID = UUID.fromString("8f0e0d0c-0b0a-0908-0706-050403020102")
    }

    // Stările posibile ale conexiunii BLE
    enum class ConnectionState {
        DISCONNECTED,
        CONNECTING,
        CONNECTED,
        DISCONNECTING,
        ERROR
    }

    // Flow-uri publice pentru observarea stării și a mesajelor
    private val _connectionState = MutableStateFlow(ConnectionState.DISCONNECTED)
    val connectionState: StateFlow<ConnectionState> = _connectionState

    private val _statusMessage = MutableStateFlow<String?>(null)
    val statusMessage: StateFlow<String?> = _statusMessage

    // Referințe interne pentru conexiune
    private var bluetoothGatt: BluetoothGatt? = null
    private var commandCharacteristic: BluetoothGattCharacteristic? = null
    private var statusCharacteristic: BluetoothGattCharacteristic? = null

    // Callback pentru evenimentele GATT
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            when (newState) {
                BluetoothProfile.STATE_CONNECTED -> {
                    Log.i(TAG, "Conectat la dispozitivul GATT: ${gatt.device.address}")
                    _connectionState.value = ConnectionState.CONNECTED
                    // După conectare, descoperim serviciile
                    gatt.discoverServices()
                }
                BluetoothProfile.STATE_DISCONNECTED -> {
                    Log.i(TAG, "Deconectat de la dispozitivul GATT")
                    _connectionState.value = ConnectionState.DISCONNECTED
                    bluetoothGatt?.close()
                    bluetoothGatt = null
                    commandCharacteristic = null
                    statusCharacteristic = null
                }
                BluetoothProfile.STATE_CONNECTING -> {
                    Log.i(TAG, "Conectare la dispozitivul GATT...")
                    _connectionState.value = ConnectionState.CONNECTING
                }
                BluetoothProfile.STATE_DISCONNECTING -> {
                    Log.i(TAG, "Deconectare de la dispozitivul GATT...")
                    _connectionState.value = ConnectionState.DISCONNECTING
                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Servicii GATT descoperite")
                
                // Căutăm serviciul și caracteristica pentru comenzi
                val service = gatt.getService(SERVICE_UUID)
                if (service != null) {
                    commandCharacteristic = service.getCharacteristic(COMMAND_CHARACTERISTIC_UUID)
                    statusCharacteristic = service.getCharacteristic(STATUS_CHARACTERISTIC_UUID)
                    
                    if (commandCharacteristic != null) {
                        Log.i(TAG, "Caracteristica pentru comenzi găsită")
                        _statusMessage.value = "Dispozitiv gata pentru comenzi"
                        // Activăm notificările pentru caracteristica de status
                        statusCharacteristic?.let { statusChar ->
                            gatt.setCharacteristicNotification(statusChar, true)
                            val cccd = statusChar.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"))
                            cccd?.let {
                                it.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                                gatt.writeDescriptor(it)
                            }
                            Log.i(TAG, "Notificările pentru status au fost activate")
                        }
                    } else {
                        Log.e(TAG, "Caracteristica pentru comenzi nu a fost găsită")
                        _statusMessage.value = "Eroare: Caracteristica pentru comenzi nu există"
                    }
                } else {
                    Log.e(TAG, "Serviciul necesar nu a fost găsit")
                    _statusMessage.value = "Eroare: Serviciul nu există"
                }
            } else {
                Log.e(TAG, "Eroare la descoperirea serviciilor: $status")
                _statusMessage.value = "Eroare la descoperirea serviciilor"
            }
        }

        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                Log.i(TAG, "Comandă trimisă cu succes")
                _statusMessage.value = "Comandă trimisă cu succes"
            } else {
                Log.e(TAG, "Eroare la trimiterea comenzii: $status")
                _statusMessage.value = "Eroare la trimiterea comenzii"
            }
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic
        ) {
            // Aici primim date de la dispozitiv (posibile notificări despre stare)
            val value = characteristic.value
            val message = value.toString(Charsets.UTF_8)
            Log.i(TAG, "Date primite de la dispozitiv: $message")
            _statusMessage.value = message
        }
    }

    /**
     * Conectare la un dispozitiv BLE
     */
    fun connect(device: BluetoothDevice) {
        Log.i(TAG, "Încercare de conectare la dispozitivul: ${device.address}")
        _connectionState.value = ConnectionState.CONNECTING
        
        // Deconectare de la orice conexiune existentă
        bluetoothGatt?.close()
        
        // Inițiere conexiune nouă
        bluetoothGatt = device.connectGatt(context, false, gattCallback)
    }

    /**
     * Deconectare de la dispozitivul curent
     */
    fun disconnect() {
        bluetoothGatt?.let { gatt ->
            Log.i(TAG, "Deconectare de la dispozitivul: ${gatt.device.address}")
            _connectionState.value = ConnectionState.DISCONNECTING
            gatt.disconnect()
        }
    }

    /**
     * Trimiterea unei comenzi către dispozitiv
     * @param command Comanda de trimis (format JSON sau text)
     * @return true dacă comanda a fost trimisă, false în caz contrar
     */
    fun sendCommand(command: String): Boolean {
        val characteristic = commandCharacteristic
        val gatt = bluetoothGatt
        
        if (characteristic == null || gatt == null || _connectionState.value != ConnectionState.CONNECTED) {
            Log.e(TAG, "Nu se poate trimite comanda: Nu există conexiune sau caracteristică")
            _statusMessage.value = "Eroare: Nu există conexiune"
            return false
        }
        
        // Setăm valoarea comenzii
        characteristic.setValue(command.toByteArray(Charsets.UTF_8))
        
        // Trimitem comanda
        val success = gatt.writeCharacteristic(characteristic)
        
        if (success) {
            Log.i(TAG, "Comandă trimisă: $command")
        } else {
            Log.e(TAG, "Eroare la trimiterea comenzii: $command")
            _statusMessage.value = "Eroare la trimiterea comenzii"
        }
        
        return success
    }

    /**
     * Curățare resurse
     */
    fun close() {
        bluetoothGatt?.close()
        bluetoothGatt = null
        commandCharacteristic = null
                    statusCharacteristic = null
        _connectionState.value = ConnectionState.DISCONNECTED
    }
}
