package com.intellidrive.optimizer;

import java.io.InputStream;
import java.io.OutputStream;

public class PrecacheBoundaryMap
{
   private boolean[] map;
   
   public PrecacheBoundaryMap(boolean[] map)
   {
      this.map = map;
   }
   
   public boolean precacheBounaryAtBlock(int blockNumber)
   {
      if(blockNumber >= map.length)
      {
         return true;
      }
      
      return map[blockNumber];
   }
   
   public static PrecacheBoundaryMap load(InputStream in)
   {
      return null;
   }
   
   public static void save(OutputStream out, PrecacheBoundaryMap map)
   {
   }   
}
