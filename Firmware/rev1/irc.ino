void command_handler(IRCMessage ircMessage) {
  // PRIVMSG ignoring CTCP messages
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
    //Serial.println(message); //todo squash this to keep console quiet 
  }
  
  // reset command
  if (ircMessage.text == "!restart") {
     Badge.player.irc->sendMessage(ircMessage.parameters,  String(NICK) + " restarting in 3 seconds");
     Badge.player.irc->sendRaw("PART #dev");
     Badge.player.irc->sendRaw("QUIT");
     delay(3000);
     ESP.restart();
  }

  // uptime (in clock cycles)
   if (ircMessage.text == "!uptime") {
     Badge.player.irc->sendMessage(ircMessage.parameters, "live for "+ String(ESP.getCycleCount()) + " cycles");
  }

  // report sketch versions and size
  if (ircMessage.text == "!firmware") {
    Badge.player.irc->sendMessage(ircMessage.parameters, "hash " + ESP.getSketchMD5() + " " + ESP.getSketchSize() +" bytes");
  }  

  if (ircMessage.text == "!boot") 
  {
       //boss(BUZZER_PIN);
       xTaskNotifyIndexed(Badge.audio.task_handle, 0, HIT_SONG, eSetValueWithOverwrite); 
       Badge.player.irc->sendMessage(ircMessage.parameters, "playing boot animation");   
  }

  if (ircMessage.text.startsWith("!play ")) {
    
    String command = ircMessage.text.substring(6); // Extract the number after "!play"
    Badge.player.irc->sendMessage(ircMessage.parameters, "Playing song: " + command);
    // Do whatever you want with the parsed word (e.g., print it to Serial)
     xTaskNotifyIndexed(Badge.audio.task_handle, 0, command.toInt(),eSetValueWithOverwrite);
  }


  if(ircMessage.text == "!update")
     {
      Badge.player.irc->sendMessage(ircMessage.parameters, "attempting update on ");
      getLocalTime(&timeinfo);
      char time_buff[28];
      strftime(time_buff, sizeof(time_buff), "%a, %d %b %y %H:%M:%S", &timeinfo);
      Badge.player.irc->sendMessage(ircMessage.parameters, time_buff);
      execOTA();
      delay(10000);
      Badge.player.irc->sendMessage(ircMessage.parameters, "update failed");
     }

  
} 

void debugSentCallback(String data) {
  Serial.println(data);
}
