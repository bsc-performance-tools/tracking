#ifndef __LINKS_H__
#define __LINKS_H__

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <localkernel.h>
#include "ClusterIDs.h"
#include "ClustersInfo.h"

using std::map;
using std::ostream;
using std::pair;
using std::string;
using std::vector;

typedef ClusterID_t            ObjectID;
typedef ClustersSet_t          ObjectSet_t;
typedef ObjectSet_t::iterator ObjectSet_iterator_t;
typedef ObjectSet_iterator_t  ObjectLinks_iterator_t;

void print_objects_set( ostream & channel, ObjectSet_t &objects, string delim );
int  subset( ObjectSet_t &set1, ObjectSet_t &set2 );

/**
 * This class represents directional links of a single cluster to one or more clusters, e.g. 1 -> [ 1, 2 ]
 */
class ObjectLinks
{
  public:
    ObjectLinks(ObjectID cluster_id);
    ~ObjectLinks();

    void clear     (void);
    int  size      (void);
    void add       (ObjectID linked_cluster_id);
    void print     (void);
    void join      (ObjectLinks *object2);
    void intersect (ObjectLinks *object2);

    ObjectID     get_object();
    ObjectSet_t get_links();
    
    ObjectLinks_iterator_t begin(void);
    ObjectLinks_iterator_t end(void);

  private:
    ObjectID     ObjectID;
    ObjectSet_t Links;
};

typedef map<ObjectID, ObjectSet_t> FrameLinks_t;
typedef FrameLinks_t::iterator FrameLinks_iterator_t;

/**
 * This class represents all directional links from the objects of a single frame to another frame
 */
class FrameLinks
{
  public:
    FrameLinks();
    ~FrameLinks();

    void add   (ObjectLinks *object_links);
    void remove(ObjectID cluster_id);
    void print (void);
    void clear (void);
    int  size  (void);

    ObjectSet_t get_links( ObjectID object_id );

    FrameLinks_iterator_t begin();
    FrameLinks_iterator_t end();

  private:
    FrameLinks_t Links;
};

typedef vector< pair<ObjectSet_t, ObjectSet_t> > DoubleLinks_t;
typedef DoubleLinks_t::iterator DoubleLinks_iterator_t;

/**
 * This class represents bidirectional links between two frames
 */
class DoubleLinks
{
  public:
    DoubleLinks (void);
    DoubleLinks (FrameLinks *Forward, FrameLinks *Backward);
    ~DoubleLinks(void);

    int  size (void);
    void add  (ObjectSet_t LeftGroup, ObjectSet_t RightGroup);
    void print(void);

    void Sort (void);
    void Merge();
    DoubleLinks * Split(DoubleLinks *Specialized);
    DoubleLinks * Reverse();

    DoubleLinks_iterator_t find( ObjectSet_t from );
    DoubleLinks_iterator_t begin();
    DoubleLinks_iterator_t end();

    DoubleLinks *GetUnivocal();
    ObjectSet_t GetFrom( DoubleLinks_iterator_t it );
    ObjectSet_t GetTo( DoubleLinks_iterator_t it );
    ObjectSet_t GetFrom( int index );
    ObjectSet_t GetTo( int index );
    ObjectSet_t get_links( ObjectSet_t );

    ObjectSet_t FindLink( ObjectSet_t &LeftGroup );

  private:
    DoubleLinks_t Links;
};

typedef vector<ObjectSet_t>        ObjectSequence_t;
typedef ObjectSequence_t::iterator ObjectSequence_iterator_t;
typedef vector<ObjectSequence_t>   SequenceLink_t;
typedef SequenceLink_t::iterator    SequenceLink_iterator_t;

/**
 * This class represents bidirectionall links across ALL frames
 */
class SequenceLink
{
  public:
    SequenceLink(vector<DoubleLinks *> &AllPairs, vector<ClustersInfo *> &clusters_info_data, double time_threshold);

    int  GetTranslationTable(int trace, int total_clusters, map< TTypeValuePair, TTypeValuePair > &TranslationTable);

    void write(ostream &Channel, bool All, bool PrettyPrint);

  private:
    SequenceLink_t    TrackedObjects;
    SequenceLink_t    FilteredObjects;
    SequenceLink_t    UntrackedObjects;
    bool              AtLeastOneUntracked;
    double            TimeThreshold;
    int               NumFrames;

    void ComputeGlobalSequences(vector<DoubleLinks *> &AllPairs);
    void Merge();
    void Merge(int s1, int s2);
    void FindUntracked();
    void FilterByTime(double TimeThreshold);
    void write(ostream &Channel, SequenceLink_t &Sequence, string Prefix, string GroupDelimiter, string FrameDelimiter);
    void write(ostream &Channel, ObjectSequence_t &Sequence, string Prefix, string GroupDelimiter, string FrameDelimiter);


    vector<ClustersInfo *> &ClustersInfoData;
};

#endif /* __LINKS_H__ */
