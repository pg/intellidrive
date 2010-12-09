package com.intellidrive.iscsi;

import java.util.UUID;

import org.jscsi.layer.iscsi.ISCSIPortal;
import org.jscsi.layer.iscsi.ISCSITargetService;

public class IntellidriveISCSITarget
{   
   private static final String LOCALHOST_ADDRESS = "127.0.0.1";
   private static final int ISCSI_PORT = 3260;
      
   public static void main(String args[]) throws Exception
   {
      ISCSITargetService targetService = new ISCSITargetService();      

      targetService.load(
         "iqn.2009-04.com.intellidrive", 
         UUID.randomUUID(), 
         new IntellidriveBlockDeviceController());

      ISCSIPortal portal = new ISCSIPortal(
         LOCALHOST_ADDRESS, ISCSI_PORT, 
         LOCALHOST_ADDRESS, ISCSI_PORT, 
         8);
      
      portal.start();
      portal.add(targetService);      
   }
}
