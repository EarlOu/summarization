#include "OnlineCluster.h"
#include "OnlineClusterBsas.h"
#include "OnlineClusterMog.h"
#include "util.h"

CV_INIT_ALGORITHM(OnlineClusterBsas, "OnlineCluster.BSAS",
    obj.info()->addParam(obj, "featureDiff", obj._featureDiff);
    obj.info()->addParam(obj, "diffThreshold", obj._diffTh);
    obj.info()->addParam(obj, "learningRate", obj._learningRate);
    obj.info()->addParam(obj, "maxNumCluster", obj._maxNumCluster);
    obj.info()->addParam(obj, "lastSeenTh", obj._lastSeenTh);
    obj.info()->addParam(obj, "histReplaceTh", obj._histReplaceTh);
    obj.info()->addParam(obj, "backgroundHistTh", obj._backgroundHistTh);
    obj.info()->addParam(obj, "mergeTh", obj._mergeTh));

CV_INIT_ALGORITHM(OnlineClusterMog, "OnlineCluster.MOG", );

bool initModule_OnlineCluster()
{
    Ptr<Algorithm> p = createOnlineClusterBsas();
    if (p->info() == NULL) return false;

    p = createOnlineClusterMog();
    if (p->info() == NULL) return false;

    return true;
}