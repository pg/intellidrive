package com.intellidrive.optimizer;

import java.io.File;
import java.util.List;

public interface DriveOptimizer
{
   public static class OptimizerResult
   {
      private BlockMap blockMap;
      private PrecacheBoundaryMap precacheBoundaryMap;

      public OptimizerResult(
         BlockMap blockMap,
         PrecacheBoundaryMap precacheBoundaryMap)
      {
         this.blockMap = blockMap;
         this.precacheBoundaryMap = precacheBoundaryMap;
      }

      public BlockMap getBlockMap()
      {
         return blockMap;
      }

      public PrecacheBoundaryMap getPrecacheBoundaryMap()
      {
         return precacheBoundaryMap;
      }
   }

   public OptimizerResult optimize(
      BlockMap blockMap,
      PrecacheBoundaryMap precacheBoundaryMap,
      List<AccessLog> accessLogs) throws Exception;

}
