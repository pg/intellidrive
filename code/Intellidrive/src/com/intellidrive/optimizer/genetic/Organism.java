package com.intellidrive.optimizer.genetic;

import com.intellidrive.optimizer.BlockMap;
import com.intellidrive.optimizer.PrecacheBoundaryMap;

public class Organism implements Comparable<Organism>
{
   private BlockMap blockMap;
   private PrecacheBoundaryMap precacheBoundaryMap;
   private float fitness;
   
   public Organism(BlockMap blockMap, PrecacheBoundaryMap precacheBoundaryMap)
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
   
   public float getFitness()
   {
      return fitness;
   }
   
   public void setFitness(float fitness)
   {
      this.fitness = fitness;
   }

   @Override
   public int compareTo(Organism otherOrganism)
   {      
      if(fitness == otherOrganism.fitness)
      {
         return 0;
      }
      
      return (fitness > otherOrganism.fitness) ? 1 : -1;
   }
}
