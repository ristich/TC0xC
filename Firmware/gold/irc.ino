void command_handler(IRCMessage ircMessage) {
  // PRIVMSG ignoring CTCP messages
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
    Serial.println(message);
  }
  
  // reset command
  if (ircMessage.text == "!restart") {
     client.sendMessage(ircMessage.parameters,  String(NICK) + " restarting in 3 seconds");
     client.sendRaw("PART #dev");
     client.sendRaw("QUIT");
     delay(3000);
     ESP.restart();
  }

  // uptime (in clock cycles)
   if (ircMessage.text == "!uptime") {
     client.sendMessage(ircMessage.parameters, "live for "+ String(ESP.getCycleCount()) + " cycles");
  }

  // report sketch versions and size
  if (ircMessage.text == "!firmware") {
    client.sendMessage(ircMessage.parameters, "hash " + ESP.getSketchMD5() + " " + ESP.getSketchSize() +" bytes");
  }  

  if (ircMessage.text == "!boot") 
  {
       //boss(BUZZER_PIN);
       xTaskNotifyIndexed(Badge.audio.task_handle, 0, HIT_SONG, eSetValueWithOverwrite); 
       client.sendMessage(ircMessage.parameters, "playing boot animation");   
  }

  // if (ircMessage.text == "!roll") 
  // {
  //      rollEm(tcleds, 0);
  //      client.sendMessage(ircMessage.parameters, "keep on rollin' babeh");   
  // }



  if(ircMessage.text == "!update")
     {
      client.sendMessage(ircMessage.parameters, "attempting update on ");
      getLocalTime(&timeinfo);
      char time_buff[28];
      strftime(time_buff, sizeof(time_buff), "%a, %d %b %y %H:%M:%S", &timeinfo);
      client.sendMessage(ircMessage.parameters, time_buff);
      execOTA();
      delay(10000);
      client.sendMessage(ircMessage.parameters, "update failed");
     }

  
} 

void debugSentCallback(String data) {
  Serial.println(data);
}
