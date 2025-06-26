









void Tasks_init() {
  queue = xQueueCreate(5, sizeof(char[50]));     // coadă pentru mesaje inter-task

  xTaskCreate(obstacleDetectionTask, "Obstacle", 2048, NULL, 1, NULL);
  xTaskCreate(buzzerTask,            "Buzzer",   2048, NULL, 1, NULL);
}










