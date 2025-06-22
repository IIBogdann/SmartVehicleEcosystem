package com.example.smarttrafficsigns.ui

import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TagWriteScreen(
    deviceName: String,
    onExit: () -> Unit
) {
    Scaffold(topBar = {
        TopAppBar(
            title = { Text("Tag Write – $deviceName") },
            navigationIcon = {
                IconButton(onClick = onExit) {
                    Icon(Icons.Default.ArrowBack, contentDescription = "Back")
                }
            }
        )
    }) { padding ->
        Column(
            modifier = Modifier
                .padding(padding)
                .padding(16.dp)
                .fillMaxSize(),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text("În modul de scriere RFID. Implementare UI urmează…")
            Spacer(modifier = Modifier.weight(1f))
        }
    }
}
