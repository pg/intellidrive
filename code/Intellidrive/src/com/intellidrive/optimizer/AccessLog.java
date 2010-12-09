package com.intellidrive.optimizer;

import java.io.DataInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class AccessLog
{
   public enum AccessType
   {
      READ, WRITE
   }
   
   public static class AccessLogEntry
   {
      private int blockNumber;
      private int time;
      private AccessType accessType;
      
      public AccessLogEntry(int blockNumber, int time, AccessType accessType)
      {
         this.blockNumber = blockNumber;
         this.time = time;
         this.accessType = accessType;
      }
      
      public int getBlockNumber()
      {
         return blockNumber;
      }
      
      public int getTime()
      {
         return time;
      }
      
      public AccessType getAccessType()
      {
         return accessType;
      }      
   }
   
   private List<AccessLogEntry> log = new ArrayList<AccessLogEntry>();
   
   public List<AccessLogEntry> getLog()
   {
      return Collections.unmodifiableList(log);
   }
   
   public static AccessLog load(InputStream in) throws IOException
   {
      AccessLog accessLog = new AccessLog();
      
      DataInputStream input = new DataInputStream(in);
      
      try
      {
         while(true)
         {
            int timestamp = toBigEndian(input.readInt());
            int blockNumber = toBigEndian(input.readInt());
         
            AccessLogEntry entry = new AccessLogEntry(blockNumber, timestamp, AccessType.READ);
            accessLog.log.add(entry);
         }
      } catch (EOFException e)
      {
      }
      
      return accessLog;
   }
   
   public static void save(OutputStream out, AccessLog log)
   {      
   }
   
   public final static int toBigEndian(int v) 
   {
      return  (v >>> 24) | (v << 24) | ((v << 8) & 0x00FF0000) | ((v >> 8) & 0x0000FF00);
   }
   
}
