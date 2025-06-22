package com.example.smarttrafficsigns.ui

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Close
import com.example.smarttrafficsigns.ble.BleConnection

/**
 * Ecran pentru controlul semnelor de circulație după conectare
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SignControlScreen(
    deviceName: String,
    connectionState: BleConnection.ConnectionState?,
    statusMessage: String,
    onSendCommand: (String) -> Boolean,
    onDisconnect: () -> Unit,
    onBack: () -> Unit
) {
    val possibleCommands = remember {
        listOf("STOP", "YIELD", "SPEED_LIMIT_30", "SPEED_LIMIT_50")
    }
    
    var selectedCommand by remember { mutableStateOf(possibleCommands.first()) }
    var customCommand by remember { mutableStateOf("") }
    var useCustomCommand by remember { mutableStateOf(false) }

    // Text românesc pentru starea conexiunii
    val connectionStatusText = when (connectionState) {
        BleConnection.ConnectionState.CONNECTED -> "Conectat"
        BleConnection.ConnectionState.CONNECTING -> "Se conectează…"
        BleConnection.ConnectionState.DISCONNECTED -> "Deconectat"
        BleConnection.ConnectionState.DISCONNECTING -> "Se deconectează…"
        BleConnection.ConnectionState.ERROR -> "Eroare"
        null -> "Necunoscut"
    }

    // Etichete și descrieri în limba română pentru comenzi
    val commandLabels = mapOf(
        "STOP" to "STOP",
        "YIELD" to "Cedează trecerea",
        "SPEED_LIMIT_30" to "Limită 30 km/h",
        "SPEED_LIMIT_50" to "Limită 50 km/h"
    )

    val commandDescriptions = mapOf(
        "STOP" to "Semn de oprire obligatorie",
        "YIELD" to "Cedează trecerea",
        "SPEED_LIMIT_30" to "Limită de viteză 30 km/h",
        "SPEED_LIMIT_50" to "Limită de viteză 50 km/h"
    )
    
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Controlul semnului: $deviceName") },
                navigationIcon = {
                    IconButton(onClick = onBack) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Înapoi la lista de dispozitive")
                    }
                },
                actions = {
                    IconButton(onClick = onDisconnect) {
                        Icon(Icons.Default.Close, contentDescription = "Deconectează")
                    }
                }
            )
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .padding(paddingValues)
                .padding(16.dp)
                .fillMaxSize()
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            // Status card
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(
                    containerColor = if (connectionState == BleConnection.ConnectionState.CONNECTED)
                        Color(0xFF4CAF50) else MaterialTheme.colorScheme.surface,
                    contentColor = if (connectionState == BleConnection.ConnectionState.CONNECTED)
                        Color.White else MaterialTheme.colorScheme.onSurface
                )
            ) {
                Column(
                    modifier = Modifier
                        .padding(16.dp)
                        .fillMaxWidth(),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Text(
                        text = "Status:",
                        style = MaterialTheme.typography.bodyMedium,
                        fontWeight = FontWeight.Bold
                    )
                    Text(
                        text = "Conexiune: $connectionStatusText",
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (statusMessage.isNotEmpty()) {
                        Text(
                            text = "Mesaj: $statusMessage",
                            style = MaterialTheme.typography.bodyMedium
                        )
                    }
                }
            }
            
            // Command selector
            Text(
                text = "Selectează un semn de circulație:",
                style = MaterialTheme.typography.titleMedium
            )
            
            // Lista de comenzi predefinite
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                possibleCommands.forEach { command ->
                    FilterChip(
                        modifier = Modifier.fillMaxWidth(),
                        selected = !useCustomCommand && selectedCommand == command,
                        onClick = {
                            useCustomCommand = false
                            selectedCommand = command
                        },
                        label = { Text(text = commandLabels[command] ?: command) }
                    )
                }
                
                FilterChip(
                        modifier = Modifier.fillMaxWidth(),
                    selected = useCustomCommand,
                    onClick = { useCustomCommand = true },
                    label = { Text(text = "Personalizat") }
                )
            }
            
            // Custom command input
            if (useCustomCommand) {
                OutlinedTextField(
                    value = customCommand,
                    onValueChange = { customCommand = it },
                    label = { Text("Comandă personalizată") },
                    modifier = Modifier.fillMaxWidth()
                )
            }
            
            // Send button
            Button(
                onClick = {
                    val commandToSend = if (useCustomCommand) customCommand else selectedCommand
                    if (commandToSend.isNotEmpty()) {
                        onSendCommand(commandToSend)
                    }
                },
                enabled = connectionState == BleConnection.ConnectionState.CONNECTED,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
            ) {
                Text(text = "Trimite comandă")
            }
            
            // Reset button appears if ACCIDENT detected
            val showReset = remember(statusMessage) {
                val low = statusMessage.lowercase()
                low.contains("accid") || low.contains("accident")
            }
            if (showReset) {
                Button(
                    onClick = { onSendCommand("RESET_ACCIDENT") },
                    colors = ButtonDefaults.buttonColors(containerColor = Color.Red),
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp)
                ) {
                    Icon(Icons.Default.Refresh, contentDescription = "Resetare")
                    Spacer(Modifier.width(8.dp))
                    Text("RESETARE SEMN")
                }
            }
            
            // Status pentru fiecare semn posibil
            Text(
                text = "Descrieri semne:",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.padding(top = 16.dp)
            )
            
            Card(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    commandDescriptions.forEach { (key, desc) ->
                        Text(text = "${commandLabels[key]} - $desc")
                    }
                    Text(text = "Pentru comenzi personalizate, folosiți formatul JSON: " +
                            "{ \"command\": \"COMMAND_NAME\", \"params\": { \"key\": \"value\" } }")
                }
            }
        }
    }
}
