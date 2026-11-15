package com.example.smarttrafficsigns.ui

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.smarttrafficsigns.ble.SppConnection

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ElysiumConnectScreen(
    deviceName: String,
    connectionState: SppConnection.State,
    statusMessage: String?,
    onDisconnect: () -> Unit,
    onBack: () -> Unit
) {
    Scaffold(topBar = {
        TopAppBar(
            title = { Text(text = "Elysium RC") },
            navigationIcon = {
                IconButton(onClick = onBack) {
                    Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                }
            }
        )
    }) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(text = "Device: $deviceName", style = MaterialTheme.typography.titleMedium)
            Text(text = "State: ${connectionState.name}")
            if (!statusMessage.isNullOrBlank()) {
                Text(text = statusMessage!!, style = MaterialTheme.typography.bodyMedium)
            }
            Spacer(modifier = Modifier.weight(1f))
            Button(onClick = onDisconnect, enabled = connectionState == SppConnection.State.CONNECTED) {
                Text(text = "Disconnect")
            }
        }
    }
}
