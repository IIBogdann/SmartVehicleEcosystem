package com.example.smarttrafficsigns.ui

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TagWriteScreen(
    deviceName: String,
    writeEnabled: Boolean,
    statusMessage: String,
    onWriteTag: () -> Boolean,
    onExit: () -> Unit
) {
    var lastResult by remember { mutableStateOf<String?>(null) }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Scriere RFID – $deviceName") },
                navigationIcon = {
                    IconButton(onClick = onExit) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Înapoi")
                    }
                }
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .padding(padding)
                .padding(16.dp)
                .fillMaxSize(),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            // Afișăm ultimul mesaj primit de la dispozitiv (poate conține conținutul vechi al tag-ului)
            if (statusMessage.isNotBlank()) {
                Card(modifier = Modifier.fillMaxWidth()) {
                    Column(Modifier.padding(16.dp)) {
                        Text(text = "Mesaj dispozitiv:")
                        Text(text = statusMessage, style = MaterialTheme.typography.bodyMedium)
                    }
                }
            }

            // Rezultat local după apăsarea butonului
            lastResult?.let {
                Text(text = it, color = MaterialTheme.colorScheme.primary)
            }

            Spacer(modifier = Modifier.weight(1f))

            Button(
                onClick = {
                    val ok = onWriteTag()
                    lastResult = if (ok) "Comanda de scriere a fost trimisă" else "Eroare la trimiterea comenzii"
                },
                enabled = writeEnabled,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
            ) {
                Text("SCRIE TAG")
            }
        }
    }
}
