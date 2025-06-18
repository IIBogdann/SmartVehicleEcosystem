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
import androidx.compose.ui.unit.dp
import com.example.smarttrafficsigns.ble.BleConnection

/**
 * Ecran pentru controlul semnelor de circulație după conectare
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SignControlScreen(
    deviceName: String,
    connectionState: BleConnection.ConnectionState,
    statusMessage: String,
    onSendCommand: (String) -> Boolean,
    onDisconnect: () -> Unit
) {
    val possibleCommands = remember {
        listOf("STOP", "YIELD", "SPEED_LIMIT_30", "SPEED_LIMIT_50", "NO_ENTRY", "ACCIDENT_AHEAD")
    }
    
    var selectedCommand by remember { mutableStateOf(possibleCommands.first()) }
    var customCommand by remember { mutableStateOf("") }
    var useCustomCommand by remember { mutableStateOf(false) }
    
    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Controlul semnului: $deviceName") },
                navigationIcon = {
                    IconButton(onClick = onDisconnect) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Înapoi la lista de dispozitive")
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
            Card(modifier = Modifier.fillMaxWidth()) {
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
                        text = "Conexiune: ${connectionState.name}",
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
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 8.dp),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                possibleCommands.forEach { command ->
                    FilterChip(
                        selected = !useCustomCommand && selectedCommand == command,
                        onClick = {
                            useCustomCommand = false
                            selectedCommand = command
                        },
                        label = { Text(text = command.replace("_", " ")) }
                    )
                }
                
                FilterChip(
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
                    Text(text = "STOP - Semn de oprire obligatorie")
                    Text(text = "YIELD - Cedează trecerea")
                    Text(text = "SPEED_LIMIT_30 - Limită de viteză 30 km/h")
                    Text(text = "SPEED_LIMIT_50 - Limită de viteză 50 km/h")
                    Text(text = "NO_ENTRY - Accesul interzis")
                    Text(text = "ACCIDENT_AHEAD - Accident în față")
                    Text(text = "Pentru comenzi personalizate, folosiți formatul JSON: " +
                            "{ \"command\": \"COMMAND_NAME\", \"params\": { \"key\": \"value\" } }")
                }
            }
        }
    }
}
