#include "dbscan/cell.h"
#include "dbscan/grid.h"
#include "dbscan/point.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory.h>

TEST(testGrid, construction) {
  static const int dim = 2;
  using pointT = point<dim>;
  using gridT = grid<dim, pointT>;
  typedef cell<dim, pointT> cellT;

  int n = 9;

  std::unique_ptr<double[]> data(new double[n * dim]);

  pointT* PRead = (pointT*) data.get();

  data[0] = 0.5; data[1] = 3.5;
  data[2] = 0; data[3] = 0;
  data[4] = 0.5; data[5] = 2.5;
  data[6] = 1.5; data[7] = 3.5;
  data[8] = 1.5; data[9] = 0.5;
  data[10] = 2.5; data[11] = 1.5;
  data[12] = 2.5; data[13] = 0.5;
  data[14] = 2.5; data[15] = 0.5;
  data[16] = 0.5; data[17] = 3.5;

  std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1));
  std::unique_ptr<intT[]> I(new intT[n]);
  std::unique_ptr<pointT[]> P(new pointT[n]);
  G->insertParallel(PRead, P.get(), n, I.get());

  EXPECT_EQ(G->numCell(), 7);
}

TEST(testGrid, basic) {
  static const int dim = 2;
  using pointT = point<dim>;
  using gridT = grid<dim, pointT>;
  typedef cell<dim, pointT> cellT;

  int n = 4;

  std::unique_ptr<double[]> data(new double[n * dim]);

  data[0] = 0; data[1] = 0;
  data[2] = 0.5; data[3] = 0;
  data[4] = 0; data[5] = 1.5;
  data[6] = 2; data[7] = 0;

  pointT* PRead = (pointT*) data.get();

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1.0));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), 1, I.get());
    EXPECT_EQ(G->numCell(), 1);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1.0));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), 2, I.get());
    EXPECT_EQ(G->numCell(), 1);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 0.5));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), 2, I.get());
    EXPECT_EQ(G->numCell(), 2);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), 3, I.get());
    EXPECT_EQ(G->numCell(), 2);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 0.5));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), 3, I.get());
    EXPECT_EQ(G->numCell(), 3);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 0.5));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), n, I.get());
    EXPECT_EQ(G->numCell(), 4);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), n, I.get());
    EXPECT_EQ(G->numCell(), 3);
  }

  {
    std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 2.01));
    std::unique_ptr<intT[]> I(new intT[n]);
    std::unique_ptr<pointT[]> P(new pointT[n]);
    G->insertParallel(PRead, P.get(), n, I.get());
    EXPECT_EQ(G->numCell(), 1);
  }
}

TEST(testGrid, countNghCell) {
  static const int dim = 2;
  using pointT = point<dim>;
  using gridT = grid<dim, pointT>;
  typedef cell<dim, pointT> cellT;

  int n = 6;

  std::unique_ptr<double[]> data(new double[n * dim]);

  pointT* PRead = (pointT*) data.get();

  data[0] = 0; data[1] = 0;
  data[2] = 2.5; data[3] = 2.5;
  data[4] = 2.5; data[5] = 3.5;
  data[6] = 2.5; data[7] = 4.5;
  data[8] = 3.5; data[9] = 3.5;
  data[10] = 3.5; data[11] = 3.5;

  std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1));
  std::unique_ptr<intT[]> I(new intT[n]);
  std::unique_ptr<pointT[]> P(new pointT[n]);
  G->insertParallel(PRead, P.get(), n, I.get());

  EXPECT_EQ(G->numCell(), 5);

  int count = 0;

  auto nbrhoodSize = [&](cellT* c) {
    count ++;
    return false;
  };

  G->nghCellMap(G->getCell(P[1].coordinate()), nbrhoodSize);

  EXPECT_EQ(count, 4);

}

TEST(testGrid, countNghPoint) {
  static const int dim = 2;
  using pointT = point<dim>;
  using gridT = grid<dim, pointT>;
  typedef cell<dim, pointT> cellT;

  int n = 6;

  std::unique_ptr<double[]> data(new double[n * dim]);

  pointT* PRead = (pointT*) data.get();

  data[0] = 0; data[1] = 0;
  data[2] = 2.5; data[3] = 2.5;
  data[4] = 2.5; data[5] = 3.5;
  data[6] = 2.5; data[7] = 4.5;
  data[8] = 3.5; data[9] = 3.5;
  data[10] = 3.5; data[11] = 3.5;

  std::unique_ptr<gridT> G(new gridT(n+1, PRead[0], 1));
  std::unique_ptr<intT[]> I(new intT[n]);
  std::unique_ptr<pointT[]> P(new pointT[n]);
  G->insertParallel(PRead, P.get(), n, I.get());

  EXPECT_EQ(G->numCell(), 5);

  int count = 0;

  auto nbrhoodSize = [&](pointT* p) {
    count ++;
    return false;
  };

  G->nghPointMap(P[1].coordinate(), nbrhoodSize);

  EXPECT_EQ(count, 5);

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
