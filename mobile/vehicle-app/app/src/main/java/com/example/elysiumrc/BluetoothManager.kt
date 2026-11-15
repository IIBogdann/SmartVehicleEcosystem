package com.example.elysiumrc

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.pm.PackageManager
import android.os.Handler
import android.os.Looper
import android.util.Log
import androidx.core.app.ActivityCompat
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.*

class BluetoothManager(private val context: Context) {
    private val bluetoothAdapter: BluetoothAdapter? = BluetoothAdapter.getDefaultAdapter()
    private var bluetoothSocket: BluetoothSocket? = null
    private var outputStream: OutputStream? = null
    private var inputStream: InputStream? = null
    private var isConnected = false
    private val handler = Handler(Looper.getMainLooper())
    private var dataCallback: ((String) -> Unit)? = null

    companion object {
        private const val TAG = "BluetoothManager"
        private val UUID_SERIAL_PORT = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
    }

    fun isBluetoothSupported(): Boolean {
        return bluetoothAdapter != null
    }

    fun isBluetoothEnabled(): Boolean {
        return bluetoothAdapter?.isEnabled == true
    }

    fun setDataCallback(callback: (String) -> Unit) {
        dataCallback = callback
    }

    fun connectToDevice(deviceName: String, onConnected: () -> Unit, onError: (String) -> Unit) {
        if (ActivityCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT)
            != PackageManager.PERMISSION_GRANTED) {
            onError("Permisiunile Bluetooth nu sunt acordate")
            return
        }

        val pairedDevices = bluetoothAdapter?.bondedDevices
        Log.d(TAG, "Dispozitive împerecheate: ${pairedDevices?.joinToString { it.name }}")

        val device = pairedDevices?.find { it.name == deviceName }

        if (device == null) {
            Log.e(TAG, "Dispozitivul $deviceName nu a fost găsit")
            onError("Dispozitivul $deviceName nu a fost găsit")
            return
        }

        Log.d(TAG, "Încercare conectare la dispozitivul: ${device.name}")

        Thread {
            try {
                bluetoothSocket = device.createRfcommSocketToServiceRecord(UUID_SERIAL_PORT)
                bluetoothSocket?.connect()
                outputStream = bluetoothSocket?.outputStream
                inputStream = bluetoothSocket?.inputStream
                isConnected = true

                Log.d(TAG, "Conectat cu succes la ${device.name}")

                handler.post {
                    onConnected()
                }

                startListening { data ->
                    Log.d(TAG, "Date primite: $data")
                }

            } catch (e: IOException) {
                Log.e(TAG, "Eroare la conectare: ${e.message}")
                handler.post {
                    onError("Eroare la conectare: ${e.message}")
                }
            }
        }.start()
    }

    fun sendCommand(command: String) {
        if (!isConnected) {
            Log.e(TAG, "Nu se poate trimite comanda: dispozitivul nu este conectat")
            return
        }

        Thread {
            try {
                Log.d(TAG, "Trimitere comandă: $command")
                outputStream?.write((command + "\n").toByteArray())
                outputStream?.flush()
                Log.d(TAG, "Comandă trimisă cu succes")
            } catch (e: IOException) {
                Log.e(TAG, "Eroare la trimiterea comenzii: ${e.message}")
            }
        }.start()
    }

    private fun startListening(onDataReceived: (String) -> Unit) {
        Thread {
            val buffer = ByteArray(1024)
            while (isConnected) {
                try {
                    val bytesRead = inputStream?.read(buffer)
                    if (bytesRead != null && bytesRead > 0) {
                        val data = String(buffer, 0, bytesRead)
                        Log.d(TAG, "Date primite brut: $data")
                        handler.post {
                            dataCallback?.invoke(data)
                            onDataReceived(data)
                        }
                    }
                } catch (e: IOException) {
                    Log.e(TAG, "Eroare la citirea datelor: ${e.message}")
                    break
                }
            }
        }.start()
    }

    fun disconnect() {
        isConnected = false
        try {
            bluetoothSocket?.close()
            Log.d(TAG, "Deconectat cu succes")
        } catch (e: IOException) {
            Log.e(TAG, "Eroare la închiderea conexiunii: ${e.message}")
        }
    }
}
