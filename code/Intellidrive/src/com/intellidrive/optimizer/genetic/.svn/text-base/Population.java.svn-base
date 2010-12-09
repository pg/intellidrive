package com.intellidrive.optimizer.genetic;

import java.util.ArrayList;

public class Population extends ArrayList<Organism>
{
   private static final long serialVersionUID = 1L;

   private float totalFitness;
   
   public void updateTotalFitness()
   {
      totalFitness = 0;
      for(Organism organism : this)
      {
         totalFitness += organism.getFitness();
      }
   }
   
   public float getTotalFitness()
   {
      return totalFitness;
   }
   
   public Organism getBestOrganism()
   {
      Organism bestOrganism = null;
      for(Organism organism : this)
      {
         if(bestOrganism == null || organism.getFitness() > bestOrganism.getFitness())
         {
            bestOrganism = organism;
         }
      }
      
      return bestOrganism;
   }
}
