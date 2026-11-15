package com.example.smarttrafficsigns.ble

import kotlinx.coroutines.flow.StateFlow

/**
 * Interfață comună pentru conexiunile către dispozitive (BLE sau Bluetooth Classic).
 * Este foarte generică momentan pentru a nu impune schimbări majore în codul existent.
 */
interface DeviceConnection {
    /** Flux cu starea conexiunii (tip intern la alegerea implementării) */
    val connectionState: StateFlow<*>

    /** Mesaj textual/status din partea dispozitivului */
    val statusMessage: StateFlow<String?>

    fun sendCommand(command: String): Boolean

    /** Deconectare asincronă – nu blochează firul principal */
    fun disconnect()

    /** Închide complet resursele (socket, gatt etc.) */
    fun close()
}
