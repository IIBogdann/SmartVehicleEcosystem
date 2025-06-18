package com.example.smarttrafficsigns.ui

import android.bluetooth.BluetoothDevice
import androidx.compose.foundation.clickable
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DeviceListScreen(
    devices: List<BluetoothDevice>,
    onRefresh: () -> Unit = {},
    onDeviceSelected: (BluetoothDevice) -> Unit = {}
) {
    Scaffold(
        topBar = {
            TopAppBar(title = { Text(text = "Semne de circulație BLE") }, actions = {
                IconButton(onClick = onRefresh) {
                    Icon(
                        imageVector = Icons.Default.Refresh,
                        contentDescription = "Re-scan"
                    )
                }
            })
        }
    ) { padding ->
        if (devices.isEmpty()) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding),
                contentAlignment = Alignment.Center
            ) {
                Text(text = "Niciun dispozitiv găsit…")
            }
        } else {
            LazyColumn(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(padding)
            ) {
                items(devices) { device ->
                    DeviceRow(device = device, onClick = { onDeviceSelected(device) })
                }
            }
        }
    }
}

@Composable
private fun DeviceRow(device: BluetoothDevice, onClick: () -> Unit) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(16.dp)
    ) {
        Text(text = device.name ?: "Dispozitiv fără nume", style = MaterialTheme.typography.bodyLarge)
        Text(text = device.address, style = MaterialTheme.typography.bodySmall)
    }
    HorizontalDivider()
}
