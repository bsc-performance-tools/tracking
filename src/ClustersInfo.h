#ifndef __CLUSTERS_INFO_H__
#define __CLUSTERS_INFO_H__

#include <string>
#include <vector>

class ClustersInfo
{
  public:
    ClustersInfo(std::string file_name);
    ~ClustersInfo();

    int GetNumClusters();
    int GetDensity( int ClusterID );
    double GetPctTotalDuration( int ClusterID ); 

  private:
    std::string         ClustersInfoFileName;
    std::vector<int>    ClustersDensity;
    std::vector<double> ClustersPctTotalDuration;

    bool ParseDensity (std::string Line);
    bool ParsePctTotalDuration (std::string Line);
};

#endif /* __CLUSTERS_INFO_H__ */
