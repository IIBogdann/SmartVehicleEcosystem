package com.example.smarttrafficsigns.ble

import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothSocket
import android.util.Log
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job

import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.isActive
import kotlinx.coroutines.launch
import java.io.BufferedReader
import java.io.InputStreamReader
import java.io.OutputStream
import java.util.UUID

/**
 * Conexiune Bluetooth Classic (Serial Port Profile) pentru Elysium RC.
 */
class SppConnection : DeviceConnection {

    companion object {
        private const val TAG = "SppConnection"
        // UUID standard pentru SPP
        val SPP_UUID: UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
    }

    private val _connectionState = MutableStateFlow(State.DISCONNECTED)
    override val connectionState: StateFlow<State> = _connectionState

    private val _statusMessage = MutableStateFlow<String?>(null)
    override val statusMessage: StateFlow<String?> = _statusMessage

    private var socket: BluetoothSocket? = null
    private var ioJob: Job? = null

    /** Stările interne ale conexiunii SPP */
    enum class State { DISCONNECTED, CONNECTING, CONNECTED, ERROR }

    /** Încearcă să se conecteze la dispozitiv (apel asincron) */
    fun connect(device: BluetoothDevice) {
        if (_connectionState.value != State.DISCONNECTED) return
        _connectionState.value = State.CONNECTING

        CoroutineScope(Dispatchers.IO).launch {
            try {
                if (device.bondState == BluetoothDevice.BOND_NONE) {
                    // opțional: device.createBond() – nu blocăm totuși
                }
                socket = device.createRfcommSocketToServiceRecord(SPP_UUID)
                socket?.connect()
                _connectionState.value = State.CONNECTED

                // Reader job
                ioJob = launch {
                    val reader = BufferedReader(InputStreamReader(socket!!.inputStream))
                    while (isActive) {
                        val line = reader.readLine() ?: break
                        Log.i(TAG, "RX: $line")
                        _statusMessage.value = line
                    }
                }
            } catch (e: Exception) {
                Log.e(TAG, "Eroare conectare SPP", e)
                _statusMessage.value = "Eroare: ${e.message}"
                _connectionState.value = State.ERROR
                close()
            }
        }
    }

    override fun sendCommand(command: String): Boolean {
        val out: OutputStream = socket?.outputStream ?: return false
        return try {
            out.write((command + "\n").toByteArray())
            out.flush()
            true
        } catch (e: Exception) {
            Log.e(TAG, "Eroare trimitere", e)
            _statusMessage.value = "Eroare: ${e.message}"
            false
        }
    }

    override fun disconnect() {
        CoroutineScope(Dispatchers.IO).launch { close() }
    }

    override fun close() {
        ioJob?.cancel()
        try { socket?.close() } catch (_: Exception) {}
        socket = null
        _connectionState.value = State.DISCONNECTED
    }
}
