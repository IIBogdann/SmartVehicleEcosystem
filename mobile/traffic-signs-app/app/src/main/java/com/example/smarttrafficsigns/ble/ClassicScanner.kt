package com.example.smarttrafficsigns.ble

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

/**
 * Scanează dispozitive Bluetooth Classic (BR/EDR) folosind *startDiscovery()* şi expune lista
 * de dispozitive descoperite printr-un StateFlow.
 *
 * Utilizare:
 *   val scanner = ClassicScanner(context)
 *   smai stiicanner.startScan()
 *   scanner.devices.collect { list -> /* afişează */ }
 *   ...
 *   scanner.close() // în onDestroy
 */
class ClassicScanner(private val context: Context) {

    private val adapter: BluetoothAdapter? = BluetoothAdapter.getDefaultAdapter()

    private val _devices = MutableStateFlow<List<BluetoothDevice>>(emptyList())
    val devices: StateFlow<List<BluetoothDevice>> = _devices

    private val receiver = object : BroadcastReceiver() {
        @SuppressLint("MissingPermission")
        override fun onReceive(ctx: Context, intent: Intent) {
            when (intent.action) {
                BluetoothDevice.ACTION_FOUND -> {
                    val device: BluetoothDevice? =
                        intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                    device?.let { addDevice(it) }
                }
                BluetoothAdapter.ACTION_DISCOVERY_FINISHED -> {
                    // Descoperirea s-a terminat; o putem reporni dacă dorim
                }
            }
        }
    }

    init {
        val filter = IntentFilter().apply {
            addAction(BluetoothDevice.ACTION_FOUND)
            addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED)
        }
        context.registerReceiver(receiver, filter)
    }

    /** Porneşte o scanare. Lista internă se golește înainte. */
    @SuppressLint("MissingPermission")
    fun startScan() {
        adapter ?: return
        if (adapter.isDiscovering) adapter.cancelDiscovery()
        _devices.value = emptyList()
        adapter.startDiscovery()
    }

    /** Opreşte scanarea, dacă este în derulare. */
    fun stopScan() {
        adapter?.takeIf { it.isDiscovering }?.cancelDiscovery()
    }

    /** Curăţare: opreşte scanarea şi deregistrează BroadcastReceiver-ul. */
    fun close() {
        stopScan()
        context.unregisterReceiver(receiver)
    }

    private fun addDevice(device: BluetoothDevice) {
        val current = _devices.value
        if (current.none { it.address == device.address }) {
            _devices.value = current + device
        }
    }
}
