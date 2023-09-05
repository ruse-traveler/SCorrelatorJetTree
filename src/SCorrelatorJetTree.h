// ----------------------------------------------------------------------------
// 'SCorrelatorJetTree.h'
// Derek Anderson
// 12.04.2022
//
// Class to construct a tree of
// jets from a specified set of
// events.
//
// Derived from code by Antonio
// Silva (thanks!!)
// ----------------------------------------------------------------------------

#ifndef SCORRELATORJETTREE_H
#define SCORRELATORJETTREE_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// standard c include
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <cstdlib>
#include <utility>
// f4a include
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
// phool includes
#include <phool/phool.h>
#include <phool/getClass.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
// main geant4 includes
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4TruthInfoContainer.h>
// jet includes
#include <jetbase/Jet.h>
#include <jetbase/JetMap.h>
#include <jetbase/JetMapv1.h>
#include <jetbase/FastJetAlgo.h>
// track evaluator includes
#include <g4eval/SvtxTrackEval.h>
#include <g4eval/SvtxEvalStack.h>
// vtx includes
#include <globalvertex/GlobalVertex.h>
#include <globalvertex/GlobalVertexMap.h>
// tracking includes
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxVertex.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxVertexMap.h>
// calo includes
#include <calobase/RawCluster.h>
#include <calobase/RawClusterUtility.h>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawTower.h>
#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerGeomContainer.h>
#include <calotrigger/CaloTriggerInfo.h>
// particle flow includes
#include <particleflowreco/ParticleFlowElement.h>
#include <particleflowreco/ParticleFlowElementContainer.h>
// fastjet includes
#include <fastjet/PseudoJet.hh>
#include <fastjet/JetDefinition.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/FunctionOfPseudoJet.hh>
// hepmc includes
#include <HepMC/GenEvent.h>
#include <HepMC/GenVertex.h>
#include <HepMC/GenParticle.h>
#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>
// root includes
#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TNtuple.h"
#include "TDirectory.h"

#pragma GCC diagnostic pop

using namespace std;
using namespace fastjet;
using namespace findNode;

// forward declarations
class TH1;
class TFile;
class TTree;
class PHG4Particle;
class PHCompositeNode;
class PHHepMCGenEvent;
class PHHepMCGenEventMap;
class PHG4TruthInfoContainer;
class Fun4AllHistoManager;
class RawClusterContainer;
class RawCluster;
class GlobalVertex;
class SvtxTrackMap;
class JetRecoEval;
class SvtxTrackEval;
class SvtxTrack;
class ParticleFlowElement;



// SCorrelatorJetTree definition ----------------------------------------------

class SCorrelatorJetTree : public SubsysReco {

  public:

    // public enums
    enum ALGO {
      ANTIKT    = 0,
      KT        = 1,
      CAMBRIDGE = 2
    };
    enum RECOMB {
      E_SCHEME   = 0,
      PT_SCHEME  = 1,
      PT2_SCHEME = 2,
      ET_SCHEME  = 3,
      ET2_SCHEME = 4
    };

    // ctor/dtor
    SCorrelatorJetTree(const string& name = "SCorrelatorJetTree", const string& outFile = "correlator_jet_tree.root", const bool isMC = false, const bool debug = false);
    ~SCorrelatorJetTree() override;

    // F4A methods
    int Init(PHCompositeNode*)          override;
    int process_event(PHCompositeNode*) override;
    int End(PHCompositeNode*)           override;

    // setters (inline)
    void SetAddTracks(const bool addTracks)    {m_addTracks      = addTracks;}
    void SetAddFlow(const bool addFlow)        {m_addFlow        = addFlow;}
    void SetAddECal(const bool addECal)        {m_addECal        = addECal;}
    void SetAddHCal(const bool addHCal)        {m_addHCal        = addHCal;}
    void SetDoQualityPlots(const bool doQA)    {m_doQualityPlots = doQA;}
    void SetRequireSiSeeds(const bool require) {m_requireSiSeeds = require;}
    void SetSaveDST(const bool doSave)         {m_saveDST        = doSave;}
    void SetIsMC(const bool isMC)              {m_isMC           = isMC;}
    void SetJetR(const double jetR)            {m_jetR           = jetR;}
    void SetJetType(const uint32_t type)       {m_jetType        = type;}
    void SetJetTreeName(const string name)     {m_jetTreeName    = name;}

    // setters (*.io.h)
    void SetParPtRange(const pair<double, double> ptRange);
    void SetParEtaRange(const pair<double, double> etaRange);
    void SetTrackPtRange(const pair<double, double> ptRange);
    void SetTrackEtaRange(const pair<double, double> etaRange);
    void SetTrackQualityRange(const pair<double, double> qualRange);
    void SetTrackNMvtxRange(const pair<double, double> nMvtxRange);
    void SetTrackNInttRange(const pair<double, double> nInttRange);
    void SetTrackNTpcRange(const pair<double, double> nTpcRange);
    void SetFlowPtRange(const pair<double, double> ptRange);
    void SetFlowEtaRange(const pair<double, double> etaRange);
    void SetECalPtRange(const pair<double, double> ptRange);
    void SetECalEtaRange(const pair<double, double> etaRange);
    void SetHCalPtRange(const pair<double, double> ptRange);
    void SetHCalEtaRange(const pair<double, double> etaRange);
    void SetJetAlgo(const ALGO jetAlgo);
    void SetRecombScheme(const RECOMB recombScheme);
    void SetJetParameters(const double rJet, const uint32_t jetType, const ALGO jetAlgo, const RECOMB recombScheme);

    // system getters
    bool   GetAddFlow()        {return m_addFlow;}
    bool   GetAddTracks()      {return m_addTracks;}
    bool   GetAddECal()        {return m_addECal;}
    bool   GetAddHCal()        {return m_addHCal;}
    bool   GetDoQualityPlots() {return m_doQualityPlots;}
    bool   GetSaveDST()        {return m_saveDST;}
    bool   GetIsMC()           {return m_isMC;}
    string GetJetTreeName()    {return m_jetTreeName;}

    // acceptance getters
    double GetEvtMinZVtx()    {return m_evtVtxRangeZ[0];}
    double GetEvtMaxZVtx()    {return m_evtVtxRangeZ[1];}
    double GetEvtMinXYVtx()   {return m_evtVtxRangeXY[0];}
    double GetEvtMaxXYVtx()   {return m_evtVtxRangeXY[1];}
    double GetParMinPt()      {return m_parPtRange[0];}
    double GetParMaxPt()      {return m_parPtRange[1];}
    double GetParMinEta()     {return m_parEtaRange[0];}
    double GetParMaxEta()     {return m_parEtaRange[1];}
    double GetTrackMinPt()    {return m_trkPtRange[0];}
    double GetTrackMaxPt()    {return m_trkPtRange[1];}
    double GetTrackMinEta()   {return m_trkEtaRange[0];}
    double GetTrackMaxEta()   {return m_trkEtaRange[1];}
    double GetTrackMinQual()  {return m_trkQualRange[0];}
    double GetTrackMaxQual()  {return m_trkQualRange[1];}
    double GetTrackMinNMvtx() {return m_trkNMvtxRange[0];}
    double GetTrackMaxNMvtx() {return m_trkNMvtxRange[1];}
    double GetTrackMinNIntt() {return m_trkNInttRange[0];}
    double GetTrackMaxNIntt() {return m_trkNInttRange[1];}
    double GetTrackMinNTpc()  {return m_trkNTpcRange[0];}
    double GetTrackMaxNTpc()  {return m_trkNTpcRange[1];}
    double GetFlowMinPt()     {return m_flowPtRange[0];}
    double GetFlowMaxPt()     {return m_flowPtRange[1];}
    double GetFlowMinEta()    {return m_flowEtaRange[0];}
    double GetFlowMaxEta()    {return m_flowEtaRange[1];}
    double GetECalMinPt()     {return m_ecalPtRange[0];}
    double GetECalMaxPt()     {return m_ecalPtRange[1];}
    double GetECalMinEta()    {return m_ecalEtaRange[0];}
    double GetECalMaxEta()    {return m_ecalEtaRange[1];}
    double GetHCalMinPt()     {return m_hcalPtRange[0];}
    double GetHCalMaxPt()     {return m_hcalPtRange[1];}
    double GetHCalMinEta()    {return m_hcalEtaRange[0];}
    double GetHCalMaxEta()    {return m_hcalEtaRange[1];}

    // jet getters
    double              GetJetR()         {return m_jetR;}
    uint32_t            GetJetType()      {return m_jetType;}
    JetAlgorithm        GetJetAlgo()      {return m_jetAlgo;}
    RecombinationScheme GetRecombScheme() {return m_recombScheme;}

  private:

    // constants
    enum CONST {
      NPart      = 2,
      NComp      = 3,
      NRange     = 2,
      NMoment    = 2,
      NInfoQA    = 4,
      NJetType   = 2,
      NCstType   = 5,
      NObjType   = 9,
      NDirectory = 6,
      NMvtxLayer = 3,
      NInttLayer = 8,
      NTpcLayer  = 48
    };

    // tracking subsytems
    enum SUBSYS {
      MVTX = 0,
      INTT = 1,
      TPC  = 2
    };

    // qa info
    enum OBJECT {
      TRACK  = 0,
      ECLUST = 1,
      HCLUST = 2,
      FLOW   = 3,
      PART   = 4,
      TJET   = 5,
      RJET   = 6,
      TCST   = 7,
      RCST   = 8
    };
    enum CST_TYPE {
      PART_CST  = 0,
      TRACK_CST = 1,
      FLOW_CST  = 2,
      ECAL_CST  = 3,
      HCAL_CST  = 4
    };
    enum INFO {
      PT  = 0,
      ETA = 1,
      PHI = 2,
      ENE = 3
    };

    // event methods (*.evt.h)
    void              GetEventVariables(PHCompositeNode* topNode);
    void              GetPartonInfo(PHCompositeNode* topNode);
    long              GetNumTrks(PHCompositeNode* topNode);
    long              GetNumChrgPars(PHCompositeNode* topNode);
    double            GetSumECalEne(PHCompositeNode* topNode);
    double            GetSumHCalEne(PHCompositeNode* topNode);
    double            GetSumParEne(PHCompositeNode* topNode);
    CLHEP::Hep3Vector GetRecoVtx(PHCompositeNode* topNode);

    // jet methods (*.jet.h)
    void FindTrueJets(PHCompositeNode* topNode);
    void FindRecoJets(PHCompositeNode* topNode);
    void AddParticles(PHCompositeNode* topNode, vector<PseudoJet>& particles, map<int, pair<Jet::SRC, int>>& fjMap);
    void AddTracks(PHCompositeNode* topNode, vector<PseudoJet>& particles, map<int, pair<Jet::SRC, int>>& fjMap);
    void AddFlow(PHCompositeNode* topNode, vector<PseudoJet>& particles, map<int, pair<Jet::SRC, int>>& fjMap);
    void AddECal(PHCompositeNode* topNode, vector<PseudoJet>& particles, map<int, pair<Jet::SRC, int>>& fjMap);
    void AddHCal(PHCompositeNode* topNode, vector<PseudoJet>& particles, map<int, pair<Jet::SRC, int>>& fjMap);

    // constituent methods (*.cst.h)
    int   GetMatchID(SvtxTrack* track);
    int   GetNumLayer(TrackSeed* seed, const uint8_t subsys = 0);
    bool  IsGoodParticle(HepMC::GenParticle* par, const bool ignoreCharge = false);
    bool  IsGoodTrack(SvtxTrack* track);
    bool  IsGoodFlow(ParticleFlowElement* flow);
    bool  IsGoodECal(CLHEP::Hep3Vector& hepVecECal);
    bool  IsGoodHCal(CLHEP::Hep3Vector& hepVecHCal);
    bool  IsOutgoingParton(HepMC::GenParticle* par);
    float GetParticleCharge(const int pid);

    // system methods (*.sys.h)
    void                          InitVariables();
    void                          InitHists();
    void                          InitTrees();
    void                          InitEvals(PHCompositeNode* topNode);
    void                          FillTrueTree();
    void                          FillRecoTree();
    void                          SaveOutput();
    void                          ResetVariables();
    int                           CreateJetNode(PHCompositeNode* topNode);
    SvtxTrackMap*                 GetTrackMap(PHCompositeNode* topNode);
    GlobalVertex*                 GetGlobalVertex(PHCompositeNode* topNode);
    HepMC::GenEvent*              GetMcEvent(PHCompositeNode* topNode);
    RawClusterContainer*          GetClusterStore(PHCompositeNode* topNode, const TString sNodeName);
    ParticleFlowElementContainer* GetFlowStore(PHCompositeNode* topNode);

    // F4A/utility members
    Fun4AllHistoManager* m_histMan   = NULL;
    SvtxEvalStack*       m_evalStack = NULL;
    SvtxTrackEval*       m_trackEval = NULL;

    // io members
    TFile*    m_outFile     = NULL;
    TTree*    m_trueTree    = NULL;
    TTree*    m_recoTree    = NULL;
    string    m_outFileName = "";
    string    m_jetTreeName = "";
    JetMapv1* m_recoJetMap  = NULL;
    JetMapv1* m_trueJetMap  = NULL;

    // tracking members
    bool isMvtxLayerHit[CONST::NMvtxLayer] = {false};
    bool isInttLayerHit[CONST::NInttLayer] = {false};
    bool isTpcLayerHit[CONST::NTpcLayer]   = {false};

    // QA members
    TH1D* m_hJetArea[CONST::NJetType];
    TH1D* m_hJetNumCst[CONST::NJetType];
    TH1D* m_hNumObject[CONST::NObjType];
    TH1D* m_hSumCstEne[CONST::NCstType];
    TH1D* m_hObjectQA[CONST::NObjType][CONST::NInfoQA];
    TH1D* m_hNumCstAccept[CONST::NCstType][CONST::NMoment];

    // system members
    bool m_doQualityPlots = true;
    bool m_requireSiSeeds = true;
    bool m_saveDST        = false;
    bool m_isMC           = true;;
    bool m_doDebug        = false;
    bool m_addTracks      = true;
    bool m_addFlow        = false;
    bool m_addECal        = false;
    bool m_addHCal        = false;

    // event acceptance parameters
    double m_evtVtxRangeZ[CONST::NRange]  = {-10., 10.};
    double m_evtVtxRangeXY[CONST::NRange] = {-5.,  5.};

    // particle acceptance parameters
    double m_parPtRange[CONST::NRange]  = {0.1,  9999.};
    double m_parEtaRange[CONST::NRange] = {-1.1, 1.1};

    // track acceptance parameters
    double m_trkPtRange[CONST::NRange]    = {0.1,  100.};
    double m_trkEtaRange[CONST::NRange]   = {-1.1, 1.1};
    double m_trkQualRange[CONST::NRange]  = {-1.,  10.};
    double m_trkNMvtxRange[CONST::NRange] = {2.,   100.};
    double m_trkNInttRange[CONST::NRange] = {1.,   100.};
    double m_trkNTpcRange[CONST::NRange]  = {35.,  100.};

    // particle flow acceptance parameters
    double m_flowPtRange[CONST::NRange]  = {0.,   9999.};
    double m_flowEtaRange[CONST::NRange] = {-1.1, 1.1};

    // calorimeter acceptance parameters
    double m_ecalPtRange[CONST::NRange]  = {0.,   9999.};
    double m_ecalEtaRange[CONST::NRange] = {-1.1, 1.1};
    double m_hcalPtRange[CONST::NRange]  = {0.,   9999.};
    double m_hcalEtaRange[CONST::NRange] = {-1.1, 1.1};

    // jet parameters
    double               m_jetR         = 0.4;
    uint32_t             m_jetType      = 0;
    JetAlgorithm         m_jetAlgo      = antikt_algorithm;
    JetDefinition*       m_trueJetDef   = NULL;
    JetDefinition*       m_recoJetDef   = NULL;
    ClusterSequence*     m_trueClust    = NULL;
    ClusterSequence*     m_recoClust    = NULL;
    RecombinationScheme  m_recombScheme = pt_scheme;

    // event, jet members
    long long         m_partonID[CONST::NPart];
    CLHEP::Hep3Vector m_partonMom[CONST::NPart];
    CLHEP::Hep3Vector m_recoVtx;
    CLHEP::Hep3Vector m_trueVtx;
    vector<PseudoJet> m_recoJets;
    vector<PseudoJet> m_trueJets;

    // output reco event variables
    unsigned long          m_recoNumJets;
    double                 m_recoVtxX;
    double                 m_recoVtxY;
    double                 m_recoVtxZ;
    double                 m_recoSumECal;
    double                 m_recoSumHCal;
    long                   m_recoNumTrks;
    // output reco jet variables
    vector<unsigned long>  m_recoJetNCst;
    vector<unsigned int>   m_recoJetID;
    vector<double>         m_recoJetE;
    vector<double>         m_recoJetPt;
    vector<double>         m_recoJetEta;
    vector<double>         m_recoJetPhi;
    vector<double>         m_recoJetArea;
    // output reco constituent variables
    vector<vector<int>>    m_recoCstMatchID;
    vector<vector<double>> m_recoCstZ;
    vector<vector<double>> m_recoCstDr;
    vector<vector<double>> m_recoCstE;
    vector<vector<double>> m_recoCstJt;
    vector<vector<double>> m_recoCstEta;
    vector<vector<double>> m_recoCstPhi;

    // output truth event variables
    unsigned long         m_trueNumJets;
    long long             m_truePartonID[CONST::NPart];
    double                m_truePartonMomX[CONST::NPart];
    double                m_truePartonMomY[CONST::NPart];
    double                m_truePartonMomZ[CONST::NPart];
    double                m_trueVtxX;
    double                m_trueVtxY;
    double                m_trueVtxZ;
    double                m_trueSumPar;
    long                  m_trueNumChrgPars;
    // output truth jet variables
    vector<unsigned long> m_trueJetNCst;
    vector<unsigned int>  m_trueJetID;
    vector<double>        m_trueJetE;
    vector<double>        m_trueJetPt;
    vector<double>        m_trueJetEta;
    vector<double>        m_trueJetPhi;
    vector<double>        m_trueJetArea;
    // output truth constituent variables
    vector<vector<int>>    m_trueCstID;
    vector<vector<double>> m_trueCstZ;
    vector<vector<double>> m_trueCstDr;
    vector<vector<double>> m_trueCstE;
    vector<vector<double>> m_trueCstJt;
    vector<vector<double>> m_trueCstEta;
    vector<vector<double>> m_trueCstPhi;

};

#endif

// end ------------------------------------------------------------------------
