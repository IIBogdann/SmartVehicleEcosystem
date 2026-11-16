package com.example.elysiumrc

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.widget.ScrollView
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import android.widget.PopupMenu
import android.graphics.Color
import android.content.res.ColorStateList
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.view.ViewCompat
import androidx.core.view.WindowInsetsCompat

class MainActivity : AppCompatActivity() {
    private val numberRegex = Regex("[-+]?\\d+(?:\\.\\d+)?")
    private lateinit var bluetoothManager: BluetoothManager
    private lateinit var statusButton: Button
    private lateinit var distanceFrontText: TextView
    private lateinit var distanceBackText: TextView
    private lateinit var distanceLeftText: TextView
    private lateinit var distanceRightText: TextView
    private lateinit var consoleText: TextView
    private lateinit var consoleScroll: ScrollView
    // Date Arduino
    private lateinit var counterText: TextView
    private lateinit var xText: TextView
    private lateinit var yText: TextView
    private lateinit var zText: TextView
    private lateinit var voltageText: TextView

    // Butoane control
    private lateinit var buttonForward: Button
    private lateinit var buttonBackward: Button
    private lateinit var buttonLeft: Button
    private lateinit var buttonRight: Button
    private lateinit var buttonStop: Button
    private lateinit var buttonMode: Button
    private lateinit var buttonAutoStart: Button
    private lateinit var buttonAutoStop: Button

    private enum class DriveMode { MANUAL, AUTO, AUTONOMOUS }
    private var currentMode = DriveMode.AUTO

    private val REQUIRED_PERMISSIONS = arrayOf(
        Manifest.permission.BLUETOOTH,
        Manifest.permission.BLUETOOTH_ADMIN,
        Manifest.permission.BLUETOOTH_CONNECT,
        Manifest.permission.BLUETOOTH_SCAN
    )

    private val requestPermissionLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ) { permissions ->
        if (permissions.all { it.value }) {
            connectToESP32()
        } else {
            Toast.makeText(this, "Permisiunile Bluetooth sunt necesare", Toast.LENGTH_LONG).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)

        statusButton = findViewById(R.id.buttonStatus)
        distanceFrontText = findViewById(R.id.distanceFrontText)
        distanceBackText = findViewById(R.id.distanceBackText)
        distanceLeftText = findViewById(R.id.distanceLeftText)
        distanceRightText = findViewById(R.id.distanceRightText)
        // Arduino data
        counterText = findViewById(R.id.counterText)
        xText = findViewById(R.id.xText)
        yText = findViewById(R.id.yText)
        zText = findViewById(R.id.zText)
        voltageText = findViewById(R.id.voltageText)

        consoleText = findViewById(R.id.consoleText)
        consoleScroll = findViewById(R.id.consoleScroll)

        buttonForward = findViewById(R.id.buttonForward)
        buttonBackward = findViewById(R.id.buttonBackward)
        buttonLeft = findViewById(R.id.buttonLeft)
        buttonRight = findViewById(R.id.buttonRight)
        buttonStop = findViewById(R.id.buttonStop)
        buttonMode = findViewById(R.id.buttonMode)
        buttonAutoStart = findViewById(R.id.buttonAutoStart)
        buttonAutoStop = findViewById(R.id.buttonAutoStop)

        bluetoothManager = BluetoothManager(this)

        setupControlButtons()
        setupModeButton()

        if (!bluetoothManager.isBluetoothSupported()) {
            updateStatus("Bluetooth nu este suportat")
            return
        }

        if (!bluetoothManager.isBluetoothEnabled()) {
            updateStatus("Te rugăm să activezi Bluetooth-ul")
            return
        }

        if (hasRequiredPermissions()) {
            connectToESP32()
        } else {
            requestPermissionLauncher.launch(REQUIRED_PERMISSIONS)
        }

        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main)) { v, insets ->
            val systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom)
            insets
        }
    }

    private fun setupControlButtons() {
        buttonForward.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> sendCommand("F")        // Accelerează înainte
                MotionEvent.ACTION_UP   -> sendCommand("N")        // Opreşte alimentarea motorului
            }
            true
        }

        buttonBackward.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> sendCommand("B")        // Merge înapoi
                MotionEvent.ACTION_UP   -> sendCommand("N")        // Neutral motor
            }
            true
        }

        buttonLeft.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> sendCommand("L")        // Viraj stânga
                MotionEvent.ACTION_UP   -> sendCommand("C")        // Aduce roţile la centru
            }
            true
        }

        buttonRight.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> sendCommand("R")        // Viraj dreapta
                MotionEvent.ACTION_UP   -> sendCommand("C")        // Centru direcţie
            }
            true
        }

        // Buton stop total (emergency)
        buttonStop.setOnClickListener {
            sendCommand("S")                                     // Stop complet (motor + direcţie)
        }
    }

    private fun setupModeButton() {
        buttonMode.setOnClickListener {
            val popup = PopupMenu(this, buttonMode)
            popup.menu.add("Manual")
            popup.menu.add("Auto")
            popup.menu.add("Autonom")
            popup.setOnMenuItemClickListener { item ->
                when (item.title) {
                    "Manual"   -> setDriveMode(DriveMode.MANUAL)
                    "Auto"     -> setDriveMode(DriveMode.AUTO)
                    "Autonom"  -> setDriveMode(DriveMode.AUTONOMOUS)
                }
                true
            }
            popup.show()
        }

        buttonAutoStart.setOnClickListener { sendCommand("AUTO_START") }
        buttonAutoStop.setOnClickListener { sendCommand("AUTO_STOP") }

        setDriveMode(currentMode)
    }

    private fun setDriveMode(mode: DriveMode) {
        currentMode = mode
        when (mode) {
            DriveMode.MANUAL -> {
                buttonMode.text = "Manual"
                buttonAutoStart.visibility = View.GONE
                buttonAutoStop.visibility = View.GONE
                sendCommand("MODE_MANUAL")
            }
            DriveMode.AUTO -> {
                buttonMode.text = "Auto"
                buttonAutoStart.visibility = View.GONE
                buttonAutoStop.visibility = View.GONE
                sendCommand("MODE_AUTO")
            }
            DriveMode.AUTONOMOUS -> {
                buttonMode.text = "Autonom"
                buttonAutoStart.visibility = View.VISIBLE
                buttonAutoStop.visibility = View.VISIBLE
                sendCommand("MODE_AUTONOMOUS")
            }
        }
    }

    private fun sendCommand(command: String) {
        bluetoothManager.sendCommand(command)
    }

    private fun hasRequiredPermissions(): Boolean {
        return REQUIRED_PERMISSIONS.all {
            ActivityCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
        }
    }

    private fun updateStatus(message: String) {
        val lower = message.lowercase()
        when {
            lower.contains("conectat") -> {
                statusButton.text = "Conectat"
                statusButton.backgroundTintList = ColorStateList.valueOf(Color.parseColor("#4CAF50"))
            }
            lower.contains("conectează") || lower.contains("conectare") -> {
                statusButton.text = "Conectare..."
                statusButton.backgroundTintList = ColorStateList.valueOf(Color.parseColor("#FFA500"))
            }
            else -> {
                statusButton.text = "Neconectat"
                statusButton.backgroundTintList = ColorStateList.valueOf(Color.parseColor("#FF0000"))
            }
        }
    }

    private fun handleIncomingData(data: String) {
        val line = data.trim()
        // Filtrare rapidă după prefix
        if (line.startsWith("\$SEN,")) {
            val nums = line.substring(5).split(",")
            if (nums.size == 4 && nums.all { numberRegex.matches(it.trim()) }) {
                runOnUiThread {
                    distanceFrontText.text = nums[0]
                    distanceBackText.text = nums[1]
                    distanceLeftText.text = nums[2]
                    distanceRightText.text = nums[3]
                }
                return
            }
        }
        if (line.startsWith("\$ARD,")) {
            val vals = line.substring(5).split(",")
            if (vals.size == 5 && vals.all { numberRegex.matches(it.trim()) }) {
                runOnUiThread {
                    counterText.text = "CNT: ${vals[0]}"
                    xText.text = "X: ${vals[1]}"
                    yText.text = "Y: ${vals[2]}"
                    zText.text = "Z: ${vals[3]}"
                    voltageText.text = "V: ${vals[4]}"
                }
                return
            }
        }

        // Dacă nu a fost consumat mai sus, trimite la consolă
        runOnUiThread {
            consoleText.append(line + "\n")
            if (consoleText.text.length > 4000) {
                consoleText.text = consoleText.text.takeLast(4000)
            }
            consoleScroll.post { consoleScroll.fullScroll(View.FOCUS_DOWN) }
        }

    }

    private fun extractNumber(raw: String): String {
        val match = numberRegex.find(raw)
        return match?.value ?: raw.trim()
    }

    private fun connectToESP32() {
        updateStatus("Se conectează...")
        bluetoothManager.connectToDevice(
            deviceName = "ElysiumRC",
            onConnected = {
                updateStatus("Conectat la ElysiumRC")
                bluetoothManager.setDataCallback { data ->
                    handleIncomingData(data)
                }
            },
            onError = { error ->
                updateStatus("Eroare: $error")
            }
        )
    }

    override fun onDestroy() {
        super.onDestroy()
        bluetoothManager.disconnect()
    }
}
