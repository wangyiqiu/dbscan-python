#include "source/geometry.h"
#include "source/geometryIO.h"
#include "source/pbbs/parallel.h"
#include "source/pbbs/gettime.h"
#include "source/pbbs/parseCommandLine.h"

#include "Caller.h"

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"[-o <outFile>] [-eps <p_epsilon>] [-minpts <p_minpts>] <inFile>");
  char* iFile = P.getArgument(0);
  char* oFile = P.getOptionValue("-o");
  size_t rounds = P.getOptionIntValue("-r",1);
  double p_epsilon = P.getOptionDoubleValue("-eps",1);
  size_t p_minpts = P.getOptionIntValue("-minpts",1);
  double p_rho = P.getOptionDoubleValue("-rho",-1);

  int dim = readHeader(iFile);
  _seq<double> PIn = readDoubleFromFile(iFile, dim);

  bool* coreFlag = (bool*) malloc(sizeof(bool) * PIn.n / dim);
  int* cluster = (int*) malloc(sizeof(bool) * PIn.n / dim);
  double* data = PIn.A;

  auto clusterer = Wrapper::Caller(data, dim, PIn.n / dim);
  clusterer.computeDBSCAN(p_epsilon, p_minpts, coreFlag, cluster);

  if (oFile != NULL) {
    writeArrayToFile("cluster-id", cluster, PIn.n / dim, oFile);
  }
  return 0;
}
