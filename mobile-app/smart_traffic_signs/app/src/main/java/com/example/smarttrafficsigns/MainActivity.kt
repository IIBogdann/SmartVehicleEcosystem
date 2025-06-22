package com.example.smarttrafficsigns

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.os.Build
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import com.example.smarttrafficsigns.NotificationUtils
import com.example.smarttrafficsigns.ble.BleConnection
import com.example.smarttrafficsigns.ble.BleScanner
import com.example.smarttrafficsigns.ui.DeviceListScreen
import com.example.smarttrafficsigns.ui.RequestPermissions
import com.example.smarttrafficsigns.ui.SignControlScreen
import com.example.smarttrafficsigns.ui.theme.SmartTrafficSignsTheme
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.isGranted
import com.google.accompanist.permissions.rememberMultiplePermissionsState

class MainActivity : ComponentActivity() {

    private lateinit var bleScanner: BleScanner
    // Gestionăm mai multe conexiuni simultane (una per dispozitiv)
    private val connections = mutableStateMapOf<String, BleConnection>()
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    @OptIn(ExperimentalPermissionsApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        bleScanner = BleScanner(this)

        setContent {
            SmartTrafficSignsTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    MainContent()
                }
            }
        }
    }

    @OptIn(ExperimentalPermissionsApi::class)
    @Composable
    private fun MainContent() {
        // Verificăm permisiunile necesare pentru BLE
        val permissionsRequired = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            listOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT,
                Manifest.permission.ACCESS_FINE_LOCATION
            )
        } else {
            listOf(
                Manifest.permission.ACCESS_FINE_LOCATION
            )
        }

        val permissionsState = rememberMultiplePermissionsState(permissionsRequired)
        val allPermissionsGranted = permissionsState.permissions.all { it.status.isGranted }

        if (allPermissionsGranted) {
            BleContent()
        } else {
            RequestPermissions(permissionsState)
        }
    }

    @Composable
    private fun BleContent() {
        // Dispozitivul pentru care afișăm ecranul de control
        var activeDevice by remember { mutableStateOf<BluetoothDevice?>(null) }

        // Starea conexiunii BLE
        val activeConn = activeDevice?.let { connections[it.address] }
        val connectionState by activeConn?.connectionState?.collectAsState() ?: remember { mutableStateOf(BleConnection.ConnectionState.DISCONNECTED) }
        val statusMessage by activeConn?.statusMessage?.collectAsState() ?: remember { mutableStateOf("") }

        // Trimite notificare dacă mesajul conține "Accident"
        LaunchedEffect(statusMessage) {
            statusMessage?.let { msg ->
                val lower = msg.lowercase()
                if (lower.contains("accident") || lower.contains("accid")) {
                    NotificationUtils.showAccidentNotification(this@MainActivity, msg)
                }
            }
        }

        // Colectăm lista de dispozitive
        val devicesList by bleScanner.devices.collectAsState()

        if (activeDevice == null) {
            // Dacă nu suntem conectați la niciun dispozitiv, afișăm lista de dispozitive
            DeviceListScreen(
                devices = devicesList,
                connectedMacs = connections.keys,
                onRefresh = { bleScanner.startScan() },
                onDeviceSelected = { device ->
                    bleScanner.stopScan()
                    // Dacă nu avem deja o conexiune pentru acest dispozitiv o creăm
                    val mac = device.address
                    val connection = connections.getOrPut(mac) { BleConnection(this@MainActivity).apply { connect(device) } }
                    activeDevice = device
                    Toast.makeText(
                        this@MainActivity,
                        "Conectare la ${device.name ?: device.address}...",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            )
        } else {
            // Dacă suntem conectați, afișăm ecranul de control
            SignControlScreen(
                deviceName = activeDevice?.name ?: activeDevice?.address ?: "Dispozitiv",
                connectionState = connectionState,
                statusMessage = statusMessage ?: "",
                onSendCommand = { command ->
                    activeDevice?.let { dev -> connections[dev.address]?.sendCommand(command) } ?: false
                },
                onDisconnect = {
                    activeDevice?.let { dev ->
                        connections[dev.address]?.disconnect()
                        connections.remove(dev.address)
                    }
                    activeDevice = null
                    bleScanner.startScan()
                },
                onBack = {
                    // Nu deconectăm, doar revenim la listă
                    activeDevice = null
                    bleScanner.startScan()
                }
            )
        }
    }

    override fun onResume() {
        super.onResume()
        // Pornim scanarea doar dacă nu suntem conectați la un dispozitiv
        bleScanner.startScan()
    }

    override fun onPause() {
        super.onPause()
        // Oprim scanarea când aplicația nu este în prim-plan pentru a economisi bateria
        bleScanner.stopScan()
    }

    override fun onDestroy() {
        super.onDestroy()
        bleScanner.stopScan()
        connections.values.forEach { it.close() }
    }
}