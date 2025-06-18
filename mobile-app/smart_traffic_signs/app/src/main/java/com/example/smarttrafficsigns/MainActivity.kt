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
    private lateinit var bleConnection: BleConnection
    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    @OptIn(ExperimentalPermissionsApi::class)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        bleScanner = BleScanner(this)
        bleConnection = BleConnection(this)

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
        // Starea conexiunii BLE
        val connectionState by bleConnection.connectionState.collectAsState()
        val statusMessage by bleConnection.statusMessage.collectAsState()

        // Ținem minte dispozitivul conectat
        var connectedDevice by remember { mutableStateOf<BluetoothDevice?>(null) }

        // Colectăm lista de dispozitive
        val devicesList by bleScanner.devices.collectAsState()

        if (connectedDevice == null) {
            // Dacă nu suntem conectați la niciun dispozitiv, afișăm lista de dispozitive
            DeviceListScreen(
                devices = devicesList,
                onRefresh = { bleScanner.startScan() },
                onDeviceSelected = { device ->
                    // Oprim scanarea și ne conectăm la dispozitiv
                    bleScanner.stopScan()
                    bleConnection.connect(device)
                    connectedDevice = device
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
                deviceName = connectedDevice?.name ?: connectedDevice?.address ?: "Dispozitiv",
                connectionState = connectionState,
                statusMessage = statusMessage ?: "",
                onSendCommand = { command -> bleConnection.sendCommand(command) },
                onDisconnect = {
                    bleConnection.disconnect()
                    connectedDevice = null
                    bleScanner.startScan()  // Reîncepem scanarea
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
        bleConnection.close()
    }
}