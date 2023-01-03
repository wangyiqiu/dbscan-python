#include "dbscan/capi.h"
#include "gtest/gtest.h"
#include "dbscan/pbbs/parallel.h"
#include <iostream>
#include <memory.h>

TEST(testDBSCAN, smallCluster1) {
  int n = 5;
  int dim = 2;

  std::unique_ptr<double[]> data(new double[n * dim]);
  std::unique_ptr<bool[]> coreFlag(new bool[n]);
  std::unique_ptr<int[]> cluster(new int[n]);

  data[0] = 0; data[1] = 2;
  data[2] = 1; data[3] = 3;
  data[4] = 1.5; data[5] = 2.5;
  data[6] = 2.5; data[7] = 1.5;
  data[8] = 4; data[9] = 0;

  DBSCAN(dim, n, data.get(), 1.42, 3, coreFlag.get(), cluster.get());

  EXPECT_EQ(coreFlag[0], 0);
  EXPECT_EQ(coreFlag[1], 1);
  EXPECT_EQ(coreFlag[2], 1);
  EXPECT_EQ(coreFlag[3], 0);
  EXPECT_EQ(coreFlag[4], 0);

  EXPECT_EQ(cluster[0], cluster[1]);
  EXPECT_EQ(cluster[0], cluster[2]);
  EXPECT_EQ(cluster[0], cluster[3]);
  EXPECT_EQ(cluster[4], -1);
}

TEST(testDBSCAN, smallCluster1_largeEps) {
  int n = 5;
  int dim = 2;

  std::unique_ptr<double[]> data(new double[n * dim]);
  std::unique_ptr<bool[]> coreFlag(new bool[n]);
  std::unique_ptr<int[]> cluster(new int[n]);

  data[0] = 0; data[1] = 2;
  data[2] = 1; data[3] = 3;
  data[4] = 1.5; data[5] = 2.5;
  data[6] = 2.5; data[7] = 1.5;
  data[8] = 4; data[9] = 0;

  DBSCAN(dim, n, data.get(), 5.7, 3, coreFlag.get(), cluster.get());

  EXPECT_EQ(coreFlag[0], 1);
  EXPECT_EQ(coreFlag[1], 1);
  EXPECT_EQ(coreFlag[2], 1);
  EXPECT_EQ(coreFlag[3], 1);
  EXPECT_EQ(coreFlag[4], 1);

  EXPECT_EQ(cluster[0], cluster[1]);
  EXPECT_EQ(cluster[0], cluster[2]);
  EXPECT_EQ(cluster[0], cluster[3]);
  EXPECT_EQ(cluster[0], cluster[4]);
}

TEST(testDBSCAN, smallCluster1_smallEps) {
  int n = 5;
  int dim = 2;

  std::unique_ptr<double[]> data(new double[n * dim]);
  std::unique_ptr<bool[]> coreFlag(new bool[n]);
  std::unique_ptr<int[]> cluster(new int[n]);

  data[0] = 0; data[1] = 2;
  data[2] = 1; data[3] = 3;
  data[4] = 1.5; data[5] = 2.5;
  data[6] = 2.5; data[7] = 1.5;
  data[8] = 4; data[9] = 0;

  DBSCAN(dim, n, data.get(), 0.7, 3, coreFlag.get(), cluster.get());

  EXPECT_EQ(coreFlag[0], 0);
  EXPECT_EQ(coreFlag[1], 0);
  EXPECT_EQ(coreFlag[2], 0);
  EXPECT_EQ(coreFlag[3], 0);
  EXPECT_EQ(coreFlag[4], 0);

  EXPECT_EQ(cluster[0], -1);
  EXPECT_EQ(cluster[1], -1);
  EXPECT_EQ(cluster[2], -1);
  EXPECT_EQ(cluster[3], -1);
  EXPECT_EQ(cluster[4], -1);
}

int main(int argc, char **argv) {
  parlay::internal::start_scheduler();
  ::testing::InitGoogleTest(&argc, argv);
  auto result = RUN_ALL_TESTS();
  parlay::internal::stop_scheduler();
  return result;
}
