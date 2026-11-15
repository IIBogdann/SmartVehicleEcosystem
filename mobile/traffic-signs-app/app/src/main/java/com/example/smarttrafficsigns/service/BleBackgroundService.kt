package com.example.smarttrafficsigns.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Binder
import android.os.Build
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat
import com.example.smarttrafficsigns.MainActivity
import com.example.smarttrafficsigns.R
import com.example.smarttrafficsigns.ble.BleConnection
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.cancel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.update

/**
 * Serviciu care rulează în fundal pentru a menține conexiunile BLE active și a gestiona notificări
 * chiar și atunci când aplicația este minimizată
 */
class BleBackgroundService : Service() {

    companion object {
        private const val TAG = "BleBackgroundService"
        private const val NOTIFICATION_ID = 1337
        private const val CHANNEL_ID = "smart_signs_service_channel"
        
        // Intents pentru controlul serviciului din afară
        const val ACTION_START_SERVICE = "com.example.smarttrafficsigns.START_SERVICE"
        const val ACTION_STOP_SERVICE = "com.example.smarttrafficsigns.STOP_SERVICE"
    }
    
    // Binder pentru conectarea clienților la serviciu
    private val binder = LocalBinder()
    
    // Scope pentru coroutine în serviciu
    private val serviceScope = CoroutineScope(Dispatchers.Default + SupervisorJob())
    
    // Flow pentru starea serviciului (număr de conexiuni active, etc)
    private val _serviceState = MutableStateFlow(ServiceState())
    val serviceState: StateFlow<ServiceState> = _serviceState
    
    // Starea serviciului - conține informații despre conexiunile active
    data class ServiceState(
        val isRunning: Boolean = false,
        val activeConnections: Int = 0,
        val accidentDetected: Boolean = false
    )

    override fun onCreate() {
        super.onCreate()
        Log.i(TAG, "Serviciu BLE creat")
    }
    
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        Log.i(TAG, "onStartCommand: ${intent?.action}")
        
        when (intent?.action) {
            ACTION_START_SERVICE -> startForegroundService()
            ACTION_STOP_SERVICE -> stopService()
        }
        
        // Dacă serviciul este oprit de sistem, îl va reporni
        return START_STICKY
    }
    
    private fun startForegroundService() {
        Log.i(TAG, "Pornire serviciu în prim-plan")
        
        // Creare canal de notificări pentru Android 8+
        createNotificationChannel()
        
        // Creare intent pentru a deschide aplicația când se apasă pe notificare
        val pendingIntent = PendingIntent.getActivity(
            this,
            0,
            Intent(this, MainActivity::class.java),
            PendingIntent.FLAG_IMMUTABLE
        )
        
        // Creare notificare persistentă
        val notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Smart Traffic Signs")
            .setContentText("Aplicația rulează în fundal și monitorizează semnele de trafic")
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentIntent(pendingIntent)
            .build()
            
        // Pornire serviciu în prim-plan cu notificarea
        startForeground(NOTIFICATION_ID, notification)
        
        _serviceState.update { it.copy(isRunning = true) }
    }
    
    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val channel = NotificationChannel(
                CHANNEL_ID,
                "Smart Traffic Signs Service",
                NotificationManager.IMPORTANCE_LOW // IMPORTANCE_LOW pentru a nu deranja utilizatorul
            ).apply {
                description = "Canal pentru serviciul de monitorizare semne de trafic"
            }
            
            val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }
    
    private fun stopService() {
        Log.i(TAG, "Oprire serviciu")
        stopForeground(true)
        stopSelf()
    }
    
    override fun onBind(intent: Intent): IBinder {
        return binder
    }
    
    /**
     * Clasa pentru comunicarea cu activitatea
     */
    inner class LocalBinder : Binder() {
        fun getService(): BleBackgroundService = this@BleBackgroundService
    }
    
    /**
     * Actualizare informații conexiune
     */
    fun updateConnectionInfo(connectionCount: Int, hasAccident: Boolean) {
        _serviceState.update { 
            it.copy(
                activeConnections = connectionCount,
                accidentDetected = hasAccident
            )
        }
        
        // Actualizare notificare cu informațiile noi
        updateNotification(connectionCount, hasAccident)
    }
    
    /**
     * Actualizare notificare cu detalii noi
     */
    private fun updateNotification(connectionCount: Int, hasAccident: Boolean) {
        val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        
        // Creare intent pentru a deschide aplicația când se apasă pe notificare
        val pendingIntent = PendingIntent.getActivity(
            this,
            0,
            Intent(this, MainActivity::class.java),
            PendingIntent.FLAG_IMMUTABLE
        )
        
        val message = if (hasAccident) {
            "ACCIDENT DETECTAT! - Semne conectate: $connectionCount"
        } else {
            "Monitorizare semne de trafic - Semne conectate: $connectionCount"
        }
        
        // Creare notificare actualizată
        val notification = NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("Smart Traffic Signs")
            .setContentText(message)
            .setSmallIcon(R.drawable.ic_launcher_foreground)
            .setContentIntent(pendingIntent)
            .setPriority(if (hasAccident) NotificationCompat.PRIORITY_HIGH else NotificationCompat.PRIORITY_LOW)
            .build()
            
        notificationManager.notify(NOTIFICATION_ID, notification)
    }
    
    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "Serviciu distrus")
        serviceScope.cancel() // Anulare toate coroutine-urile
    }
    

}
