static void send_rf_data(){
     
      for (byte i = 0; i <= RETRY_LIMIT; ++i) {  // tx and wait for ack up to RETRY_LIMIT times
//      rf12_sleep(-1);              // Wake up RF module - not needed in always-on scenario
      while (!rf12_canSend())
      rf12_recvDone();
      rf12_sendStart(RF12_HDR_ACK, &emonglcd, sizeof emonglcd); 
      rf12_sendWait(2);           // Wait for RF to finish sending while in standby mode
      byte acked = waitForAck();  // Wait for ACK
//      rf12_sleep(0);              // Put RF module to sleep - not needed in always-on scenario
      if (acked) { return; }      // Return if ACK received
      delay(RETRY_PERIOD * 10);  // Not the most elegant way
   } 
}

static byte waitForAck() {
 MilliTimer ackTimer;
 while (!ackTimer.poll(ACK_TIME)) {
   if (rf12_recvDone() && rf12_crc == 0 &&
      rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | MYNODE))
      return 1;
   }
 return 0;
}

