package com.example.smarttrafficsigns

import android.app.NotificationChannel
import android.app.NotificationManager
import android.content.Context
import android.graphics.Color
import android.os.Build
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat

object NotificationUtils {

    private const val CHANNEL_ID = "traffic_sign_alerts"
    private const val CHANNEL_NAME = "Alerte Semn Trafic"
    private const val CHANNEL_DESC = "Notificări despre evenimente rutiere primite de la semnele de trafic"

    private fun ensureChannel(context: Context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val manager = context.getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            if (manager.getNotificationChannel(CHANNEL_ID) == null) {
                val channel = NotificationChannel(CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_HIGH).apply {
                    description = CHANNEL_DESC
                    enableLights(true)
                    lightColor = Color.RED
                    enableVibration(true)
                }
                manager.createNotificationChannel(channel)
            }
        }
    }

    fun showAccidentNotification(context: Context, message: String) {
        ensureChannel(context)
        val builder = NotificationCompat.Builder(context, CHANNEL_ID)
            .setSmallIcon(android.R.drawable.stat_notify_error)
            .setContentTitle("Accident raportat")
            .setContentText(message)
            .setPriority(NotificationCompat.PRIORITY_HIGH)
            .setAutoCancel(true)
            .setVibrate(longArrayOf(0, 500, 200, 500))

        with(NotificationManagerCompat.from(context)) {
            notify(1, builder.build())
        }
    }
}
