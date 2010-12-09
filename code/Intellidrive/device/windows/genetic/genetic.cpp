/* ----------------------------------------------------------------------------
  genetic.cpp
  gebhard2 april 17, 2009

  DESCRIPTION:
   Heavily influenced by example programs from the GAlib library.  We're
   using GA2DArrayGenome and GAIncrementalGA.
---------------------------------------------------------------------------- */

#include "genetic.h"
#include <ga/ga.h>
#include <ga/GA2DArrayGenome.h>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>

namespace io = boost::iostreams;

float objective(GAGenome &);

/**
 * This is just a test!
 */
int testGeneticDriveOptimizer()
{
	GeneticDriveOptimizer* genetic = GeneticDriveOptimizer::GetInstance();
	//TODO: Add file stream_buffer parameters
	//  genetic->optimize();
	return 0;
}

GeneticDriveOptimizer::GeneticDriveOptimizer* GeneticDriveOptimizer::s_instance = NULL;

GeneticDriveOptimizer::GeneticDriveOptimizer* GeneticDriveOptimizer::GetInstance()
{
	if( s_instance == NULL )
	{
		s_instance = new GeneticDriveOptimizer();
	}
	return( s_instance );
}

GeneticDriveOptimizer::GeneticDriveOptimizer()
{
  //TODO: Use blockMap.length as first parameter
  GA2DArrayGenome<int> genome(2000, 5, objective);
  GAIncrementalGA ga(genome);
  ga.populationSize(POPULATION_SIZE);
  ga.nGenerations(NUM_GENERATIONS);
  ga.pMutation(PROB_MUTATION);
  ga.pCrossover(PROB_CROSSOVER);
  //ga.scoreFilename(SCORE_FILENAME);
  ga.flushFrequency(50);	// dump scores to disk every 50th generation
}

GeneticDriveOptimizer::~GeneticDriveOptimizer()
{
  //TODO: Clean up at disposal.
}

float objective(GAGenome & g)
{
  GA2DArrayGenome<int> & genome = (GA2DArrayGenome<int> &)g;  
  float fitness = 0;
  //TODO
  return fitness;
}

void GeneticDriveOptimizer::optimize(io::stream_buffer<io::file_sink> blockMap, io::stream_buffer<io::file_sink> precacheMap, io::stream_buffer<io::file_sink> accessLog)
{
  ga.evolve();
  //ga.evolve(seed);
}


