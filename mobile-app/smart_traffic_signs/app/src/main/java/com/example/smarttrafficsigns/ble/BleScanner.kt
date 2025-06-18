package com.example.smarttrafficsigns.ble

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.le.BluetoothLeScanner
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

/**
 * Simplu scanner BLE care expune lista de dispozitive descoperite prin StateFlow.
 * Nu filtrează după servicii; bonus: într-o versiune viitoare vom filtra după UUID-ul "TrafficSignService".
 */
class BleScanner(context: Context) {

    private val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
    private val adapter: BluetoothAdapter? = bluetoothManager.adapter
    private val bleScanner: BluetoothLeScanner? = adapter?.bluetoothLeScanner

    private val _devices = MutableStateFlow<List<BluetoothDevice>>(emptyList())
    val devices: StateFlow<List<BluetoothDevice>> get() = _devices

    private val scope = CoroutineScope(Dispatchers.Main)

    private val callback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult?) {
            result?.device?.let { device ->
                scope.launch {
                    val old = _devices.value.toMutableList()
                    if (old.none { it.address == device.address }) {
                        old.add(device)
                        _devices.value = old
                    }
                }
            }
        }
    }

    fun startScan() {
        if (adapter?.isEnabled == true) {
            bleScanner?.startScan(callback)
        }
    }

    fun stopScan() {
        bleScanner?.stopScan(callback)
    }
}
