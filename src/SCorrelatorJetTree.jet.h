// ----------------------------------------------------------------------------
// 'SCorrelatorJetTree.jets.h'
// Derek Anderson
// 01.18.2023
//
// Class to construct a tree of
// jets from a specified set of
// events.
//
// Methods relevant to jets
// are collected here.
//
// Derived from code by Antonio
// Silva (thanks!!)
// ----------------------------------------------------------------------------

#pragma once

using namespace std;
using namespace fastjet;
using namespace findNode;



// jet methods ----------------------------------------------------------------

void SCorrelatorJetTree::FindJets(PHCompositeNode *topNode) {

  // print debug statement
  if (m_doDebug) {
    cout << "SCorrelatorJetTree::FindJets(PHCompositeNode*) Finding jets..." << endl;
  }

  // declare fastjet objects
  fastjet::JetDefinition     *jetdef = new fastjet::JetDefinition(m_jetalgo, m_jetr, m_recomb_scheme, fastjet::Best);
  vector<fastjet::PseudoJet>  particles;

  // instantiate fastjet map
  map<int, pair<Jet::SRC, int>> fjMap;

  // add constitutents
  const bool doParticleFlow = m_add_particleflow;
  const bool doTracks       = m_add_tracks;
  const bool doCaloClusters = (m_add_EMCal_clusters || m_add_HCal_clusters);
  if (doParticleFlow) AddParticleFlow(topNode, particles, fjMap);
  if (doTracks)       AddTracks(topNode, particles, fjMap);
  if (doCaloClusters) AddClusters(topNode, particles, fjMap);

  // cluster jets
  fastjet::ClusterSequence   jetFinder(particles, *jetdef);
  vector<fastjet::PseudoJet> fastjets = jetFinder.inclusive_jets();
  delete jetdef;

  // prepare vectors for filling
  m_recoJetNCst.clear();
  m_recoJetId.clear();
  m_recoJetTruId.clear();
  m_recoJetE.clear();
  m_recoJetPt.clear();
  m_recoJetEta.clear();
  m_recoJetPhi.clear();
  m_recoJetArea.clear();
  m_recoCstZ.clear();
  m_recoCstDr.clear();
  m_recoCstE.clear();
  m_recoCstJt.clear();
  m_recoCstEta.clear();
  m_recoCstPhi.clear();

  // declare vectors for storing constituents
  vector<double> vecRecCstZ;
  vector<double> vecRecCstDr;
  vector<double> vecRecCstE;
  vector<double> vecRecCstJt;
  vector<double> vecRecCstEta;
  vector<double> vecRecCstPhi;
  vecRecCstZ.clear();
  vecRecCstDr.clear();
  vecRecCstE.clear();
  vecRecCstJt.clear();
  vecRecCstEta.clear();
  vecRecCstPhi.clear();

  // fill jet/constituent variables
  unsigned long nRecJet(0);
  unsigned long nRecCst(0);
  for (unsigned int iJet = 0; iJet < fastjets.size(); ++iJet) {

    // get jet info
    const unsigned int jetNCst  = fastjets[iJet].constituents().size();
    const unsigned int jetID    = iJet;
    const unsigned int jetTruID = 99999;  // FIXME: this will need to be changed to the matched truth jet
    const double       jetPhi   = fastjets[iJet].phi_std();
    const double       jetEta   = fastjets[iJet].pseudorapidity();
    const double       jetArea  = 0.;  // FIXME: jet area needs to be defined
    const double       jetE     = fastjets[iJet].E();
    const double       jetPt    = fastjets[iJet].perp();
    const double       jetPx    = fastjets[iJet].px();
    const double       jetPy    = fastjets[iJet].py();
    const double       jetPz    = fastjets[iJet].pz();
    const double       jetP     = sqrt((jetPx * jetPx) + (jetPy * jetPy) + (jetPz * jetPz));

    // clear constituent vectors
    vecRecCstZ.clear();
    vecRecCstDr.clear();
    vecRecCstE.clear();
    vecRecCstJt.clear();
    vecRecCstEta.clear();
    vecRecCstPhi.clear();

    // loop over constituents
    vector<fastjet::PseudoJet> csts = fastjets[iJet].constituents();
    for (unsigned int iCst = 0; iCst < csts.size(); ++iCst) {

      // get constituent info
      const double cstPhi = csts[iCst].phi_std();
      const double cstEta = csts[iCst].pseudorapidity();
      const double cstE   = csts[iCst].E();
      const double cstJt  = csts[iCst].perp();
      const double cstJx  = csts[iCst].px();
      const double cstJy  = csts[iCst].py();
      const double cstJz  = csts[iCst].pz();
      const double cstJ   = ((cstJx * cstJx) + (cstJy * cstJy) + (cstJz * cstJz));
      const double cstZ   = cstJ / jetP;
      const double cstDf  = cstPhi - jetPhi;
      const double cstDh  = cstEta - jetEta;
      const double cstDr  = sqrt((cstDf * cstDf) + (cstDh * cstDh));

      // add csts to vectors
      vecRecCstZ.push_back(cstZ);
      vecRecCstDr.push_back(cstDr);
      vecRecCstE.push_back(cstE);
      vecRecCstJt.push_back(cstJt);
      vecRecCstEta.push_back(cstEta);
      vecRecCstPhi.push_back(cstPhi);

      // fill QA histograms and increment counters
      m_hObjectQA[OBJECT::RCST][INFO::PT]  -> Fill(cstJt);
      m_hObjectQA[OBJECT::RCST][INFO::ETA] -> Fill(cstEta);
      m_hObjectQA[OBJECT::RCST][INFO::PHI] -> Fill(cstPhi);
      m_hObjectQA[OBJECT::RCST][INFO::ENE] -> Fill(cstE);
      ++nRecCst;
    }  // end constituent loop

    // store jet/cst output
    m_recoJetNCst.push_back(jetNCst);
    m_recoJetId.push_back(jetID);
    m_recoJetTruId.push_back(jetTruID);
    m_recoJetE.push_back(jetE);
    m_recoJetPt.push_back(jetPt);
    m_recoJetEta.push_back(jetEta);
    m_recoJetPhi.push_back(jetPhi);
    m_recoJetArea.push_back(jetArea);
    m_recoCstZ.push_back(vecRecCstZ);
    m_recoCstDr.push_back(vecRecCstDr);
    m_recoCstE.push_back(vecRecCstE);
    m_recoCstJt.push_back(vecRecCstJt);
    m_recoCstEta.push_back(vecRecCstEta);
    m_recoCstPhi.push_back(vecRecCstPhi);

    // fill QA histograms and increment counters
    m_hJetArea[1]                        -> Fill(jetArea);
    m_hJetNumCst[1]                      -> Fill(jetNCst);
    m_hObjectQA[OBJECT::RJET][INFO::PT]  -> Fill(jetPt);
    m_hObjectQA[OBJECT::RJET][INFO::ETA] -> Fill(jetEta);
    m_hObjectQA[OBJECT::RJET][INFO::PHI] -> Fill(jetPhi);
    m_hObjectQA[OBJECT::RJET][INFO::ENE] -> Fill(jetE);
    ++nRecJet;
  }  // end jet loop

  // fill QA histograms
  m_hNumObject[OBJECT::RJET] -> Fill(nRecJet);
  m_hNumObject[OBJECT::RCST] -> Fill(nRecCst);

  // store evt info
  // FIXME: replace parton branches w/ relevant info
  m_recoNumJets       = nRecJet;
  m_recoPartonID[0]   = -9999;
  m_recoPartonID[1]   = -9999;
  m_recoPartonMomX[0] = -9999.;
  m_recoPartonMomX[1] = -9999.;
  m_recoPartonMomY[0] = -9999.;
  m_recoPartonMomY[1] = -9999.;
  m_recoPartonMomZ[0] = -9999.;
  m_recoPartonMomZ[1] = -9999.;

  // fill object tree
  m_recoTree -> Fill();
  return;

}  // end 'FindJets(PHCompositeNode*)'



void SCorrelatorJetTree::FindMcJets(PHCompositeNode *topNode) {

  // print debug statement
  if (m_doDebug || (Verbosity() > 6)) {
    cout << "SCorrelatorJetTree::FindMcJets(PHCompositeNode*) Finding MC jets..." << endl;
  }

  // declare fastjet objects & mc fastjet map
  fastjet::JetDefinition        *jetdef = new fastjet::JetDefinition(m_jetalgo, m_jetr, m_recomb_scheme, fastjet::Best);
  vector<fastjet::PseudoJet>     particles;
  map<int, pair<Jet::SRC, int>>  fjMapMC;

  // add constituents
  AddParticles(topNode, particles, fjMapMC);

  // cluster jets
  fastjet::ClusterSequence   jetFinder(particles, *jetdef);
  vector<fastjet::PseudoJet> mcfastjets = jetFinder.inclusive_jets();
  delete jetdef;

  // prepare vectors for filling
  m_trueJetNCst.clear();
  m_trueJetId.clear();
  m_trueJetTruId.clear();
  m_trueJetE.clear();
  m_trueJetPt.clear();
  m_trueJetEta.clear();
  m_trueJetPhi.clear();
  m_trueJetArea.clear();
  m_trueCstZ.clear();
  m_trueCstDr.clear();
  m_trueCstE.clear();
  m_trueCstJt.clear();
  m_trueCstEta.clear();
  m_trueCstPhi.clear();

  // declare vectors to storing constituents
  vector<double> vecTruCstZ;
  vector<double> vecTruCstDr;
  vector<double> vecTruCstE;
  vector<double> vecTruCstJt;
  vector<double> vecTruCstEta;
  vector<double> vecTruCstPhi;
  vecTruCstZ.clear();
  vecTruCstDr.clear();
  vecTruCstE.clear();
  vecTruCstJt.clear();
  vecTruCstEta.clear();
  vecTruCstPhi.clear();

  // fill jets/constituent variables
  unsigned int nTruJet(0);
  unsigned int nTruCst(0);
  for (unsigned int iTruJet = 0; iTruJet < mcfastjets.size(); ++iTruJet) {

    // get jet info
    const unsigned int jetNCst  = mcfastjets[iTruJet].constituents().size();
    const unsigned int jetID    = 9999.;  // FIXME: this will need to be changed to the matched reco jet
    const unsigned int jetTruID = iTruJet;
    const double       jetPhi   = mcfastjets[iTruJet].phi_std();
    const double       jetEta   = mcfastjets[iTruJet].pseudorapidity();
    const double       jetArea  = 0.;  // FIXME: jet area needs to be defined
    const double       jetE     = mcfastjets[iTruJet].E();
    const double       jetPt    = mcfastjets[iTruJet].perp();
    const double       jetPx    = mcfastjets[iTruJet].px();
    const double       jetPy    = mcfastjets[iTruJet].py();
    const double       jetPz    = mcfastjets[iTruJet].pz();
    const double       jetP     = sqrt((jetPx * jetPx) + (jetPy * jetPy) + (jetPz * jetPz));

    // clear constituent vectors
    vecTruCstZ.clear();
    vecTruCstDr.clear();
    vecTruCstE.clear();
    vecTruCstJt.clear();
    vecTruCstEta.clear();
    vecTruCstPhi.clear();

    // loop over constituents
    vector<fastjet::PseudoJet> trueCsts = mcfastjets[iTruJet].constituents();
    for (unsigned int iTruCst = 0; iTruCst < trueCsts.size(); ++iTruCst) {

      // get constituent info
      const double cstPhi = trueCsts[iTruCst].phi_std();
      const double cstEta = trueCsts[iTruCst].pseudorapidity();
      const double cstE   = trueCsts[iTruCst].E();
      const double cstJt  = trueCsts[iTruCst].perp();
      const double cstJx  = trueCsts[iTruCst].px();
      const double cstJy  = trueCsts[iTruCst].py();
      const double cstJz  = trueCsts[iTruCst].pz();
      const double cstJ   = ((cstJx * cstJx) + (cstJy * cstJy) + (cstJz * cstJz));
      const double cstZ   = cstJ / jetP;
      const double cstDf  = cstPhi - jetPhi;
      const double cstDh  = cstEta - jetEta;
      const double cstDr  = sqrt((cstDf * cstDf) + (cstDh * cstDh));

      // add csts to vectors
      vecTruCstZ.push_back(cstZ);
      vecTruCstDr.push_back(cstDr);
      vecTruCstE.push_back(cstE);
      vecTruCstJt.push_back(cstJt);
      vecTruCstEta.push_back(cstEta);
      vecTruCstPhi.push_back(cstPhi);

      // fill QA histograms and increment counters
      m_hObjectQA[OBJECT::TCST][INFO::PT]  -> Fill(cstJt);
      m_hObjectQA[OBJECT::TCST][INFO::ETA] -> Fill(cstEta);
      m_hObjectQA[OBJECT::TCST][INFO::PHI] -> Fill(cstPhi);
      m_hObjectQA[OBJECT::TCST][INFO::ENE] -> Fill(cstE);
      ++nTruCst;
    }  // end constituent loop

    // store jet/cst output
    m_trueJetNCst.push_back(jetNCst);
    m_trueJetId.push_back(jetID);
    m_trueJetTruId.push_back(jetTruID);
    m_trueJetE.push_back(jetE);
    m_trueJetPt.push_back(jetPt);
    m_trueJetEta.push_back(jetEta);
    m_trueJetPhi.push_back(jetPhi);
    m_trueJetArea.push_back(jetArea);
    m_trueCstZ.push_back(vecTruCstZ);
    m_trueCstDr.push_back(vecTruCstDr);
    m_trueCstE.push_back(vecTruCstE);
    m_trueCstJt.push_back(vecTruCstJt);
    m_trueCstEta.push_back(vecTruCstEta);
    m_trueCstPhi.push_back(vecTruCstPhi);

    // fill QA histograms and increment counters
    m_hJetArea[0]                        -> Fill(jetArea);
    m_hJetNumCst[0]                      -> Fill(jetNCst);
    m_hObjectQA[OBJECT::TJET][INFO::PT]  -> Fill(jetPt);
    m_hObjectQA[OBJECT::TJET][INFO::ETA] -> Fill(jetEta);
    m_hObjectQA[OBJECT::TJET][INFO::PHI] -> Fill(jetPhi);
    m_hObjectQA[OBJECT::TJET][INFO::ENE] -> Fill(jetE);
    ++nTruJet;
  }  // end jet loop

  // fill QA histograms
  m_hNumObject[OBJECT::TJET] -> Fill(nTruJet);
  m_hNumObject[OBJECT::TCST] -> Fill(nTruCst);

  // store evt info
  // FIXME: grab actual parton values
  m_trueNumJets       = nTruJet;
  m_truePartonID[0]   = -9999;
  m_truePartonID[1]   = -9999;
  m_truePartonMomX[0] = -9999.;
  m_truePartonMomX[1] = -9999.;
  m_truePartonMomY[0] = -9999.;
  m_truePartonMomY[1] = -9999.;
  m_truePartonMomZ[0] = -9999.;
  m_truePartonMomZ[1] = -9999.; 

  // fill output tree
  m_trueTree -> Fill();
  return;

}  // end 'FindMcJets(PHCompositeNode*)'



void SCorrelatorJetTree::AddParticleFlow(PHCompositeNode *topNode, vector<PseudoJet> &particles, map<int, pair<Jet::SRC, int>> &fjMap) {

  // print debug statement
  if (m_doDebug) {
    cout << "SCorrelatorJetTree::AddParticleFlow(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&) Adding particle flow elements..." << endl;
  }

  // warn if jets should be charged
  if (m_doDebug && (m_jetType != 1)) {
    cerr << "SCorrelatorJetTree::AddParticleFlow - Warning - trying to add particle flow elements to charged jets!" << endl;
  }

  // declare pf  objects
  ParticleFlowElementContainer *pflowContainer = findNode::getClass<ParticleFlowElementContainer>(topNode, "ParticleFlowElements");

  // loop over pf elements
  unsigned int                                iPart     = particles.size();
  unsigned int                                nFlowTot  = 0;
  unsigned int                                nFlowAcc  = 0;
  double                                      eFlowSum  = 0.;
  ParticleFlowElementContainer::ConstRange    begin_end = pflowContainer -> getParticleFlowElements();
  ParticleFlowElementContainer::ConstIterator rtiter;
  for (rtiter = begin_end.first; rtiter != begin_end.second; ++rtiter) {

    // get pf element
    ParticleFlowElement *pflow = rtiter -> second;
    if (!pflow) {
      continue;
    } else {
      ++nFlowTot;
    }

    // check if good
    const bool isGoodElement = IsGoodParticleFlow(pflow);
    if (!isGoodElement) {
      continue;
    } else {
      ++nFlowAcc;
    }

    // create pseudojet and add to constituent vector
    const int    pfID = pflow -> get_id();
    const double pfE  = pflow -> get_e();
    const double pfPx = pflow -> get_px();
    const double pfPy = pflow -> get_py();
    const double pfPz = pflow -> get_pz();

    fastjet::PseudoJet fjPartFlow(pfPx, pfPy, pfPz, pfE);
    fjPartFlow.set_user_index(iPart);
    particles.push_back(fjPartFlow);

    // add pf element to fastjet map
    pair<int, pair<Jet::SRC, int>> jetPartFlowPair(iPart, make_pair(Jet::SRC::PARTICLE, pfID));
    fjMap.insert(jetPartFlowPair);

    // fill QA histograms, increment sums and counters
    m_hObjectQA[OBJECT::FLOW][INFO::PT]  -> Fill(fjPartFlow.perp());
    m_hObjectQA[OBJECT::FLOW][INFO::ETA] -> Fill(fjPartFlow.pseudorapidity());
    m_hObjectQA[OBJECT::FLOW][INFO::PHI] -> Fill(fjPartFlow.phi_std());
    m_hObjectQA[OBJECT::FLOW][INFO::ENE] -> Fill(fjPartFlow.E());
    eFlowSum += pfE;
    ++iPart;
  }  // end pf element loop

  // fill QA histograms
  m_hNumObject[OBJECT::FLOW]             -> Fill(nFlowAcc);  // TODO: fill this one in cst collection method
  m_hNumCstAccept[CST_TYPE::FLOW_CST][0] -> Fill(nFlowTot);
  m_hNumCstAccept[CST_TYPE::FLOW_CST][1] -> Fill(nFlowAcc);  // TODO: fill this one in jet finding method
  m_hSumCstEne[CST_TYPE::FLOW_CST]       -> Fill(eFlowSum);
  return;

}  // end 'AddParticleFlow(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&)'



void SCorrelatorJetTree::AddTracks(PHCompositeNode *topNode, vector<PseudoJet> &particles, map<int, pair<Jet::SRC, int>> &fjMap) {

  // print debug statement
  if (m_doDebug) {
    cout << "SCorrelatorJetTree::AddTracks(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&) Adding tracks..." << endl;
  }

  // get track map
  SvtxTrackMap *trackmap = findNode::getClass<SvtxTrackMap>(topNode, "SvtxTrackMap");
  if (!trackmap) {
    cerr << PHWHERE
         << "PANIC: SvtxTrackMap node is missing, can't collect tracks!"
         << endl;
    return;
  }

  // loop over tracks
  unsigned int  iPart   = particles.size();
  unsigned int  nTrkTot = 0;
  unsigned int  nTrkAcc = 0;
  double        eTrkSum = 0.;
  SvtxTrack    *track   = 0;
  for (SvtxTrackMap::Iter iter = trackmap -> begin(); iter != trackmap -> end(); ++iter) {

    // get track
    track = iter -> second;
    if (!track) {
      continue;
    } else {
      ++nTrkTot;
    }

    // check if good
    const bool isGoodTrack = IsGoodTrack(track);
    if(!isGoodTrack) {
      continue;
    } else {
      ++nTrkAcc;
    }

    // create pseudojet and add to constituent vector
    const int    trkID = track -> get_id();
    const double trkPx = track -> get_px();
    const double trkPy = track -> get_py();
    const double trkPz = track -> get_pz();
    const double trkE  = sqrt((trkPx * trkPx) + (trkPy * trkPy) + (trkPz * trkPz) + (MassPion * MassPion));

    fastjet::PseudoJet fjTrack(trkPx, trkPy, trkPz, trkE);
    fjTrack.set_user_index(iPart);
    particles.push_back(fjTrack);

    // add track to fastjet map
    pair<int, pair<Jet::SRC, int>> jetTrkPair(iPart, make_pair(Jet::SRC::TRACK, trkID));
    fjMap.insert(jetTrkPair);

    // fill QA histograms, increment sums and counters
    m_hObjectQA[OBJECT::TRACK][INFO::PT]  -> Fill(fjTrack.perp());
    m_hObjectQA[OBJECT::TRACK][INFO::ETA] -> Fill(fjTrack.pseudorapidity());
    m_hObjectQA[OBJECT::TRACK][INFO::PHI] -> Fill(fjTrack.phi_std());
    m_hObjectQA[OBJECT::TRACK][INFO::ENE] -> Fill(fjTrack.E());
    eTrkSum += trkE;
    ++iPart;
  }  // end track loop

  // fill QA histograms
  m_hNumObject[OBJECT::TRACK]             -> Fill(nTrkAcc);  // TODO: fill this one in cst collection method
  m_hNumCstAccept[CST_TYPE::TRACK_CST][0] -> Fill(nTrkTot);
  m_hNumCstAccept[CST_TYPE::TRACK_CST][1] -> Fill(nTrkAcc);  // TODO: fill this one in jet finding method
  m_hSumCstEne[CST_TYPE::TRACK_CST]       -> Fill(eTrkSum);
  return;

}  // end 'AddTracks(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&)'



void SCorrelatorJetTree::AddClusters(PHCompositeNode *topNode, vector<PseudoJet> &particles, map<int, pair<Jet::SRC, int>> &fjMap) {

  // print debug statement
  if (m_doDebug) {
    cout << "SCorrelatorJetTree::AddClusters(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&) Adding clusters..." << endl;
  }

  // warn if jets should be charged
  if (m_doDebug && (m_jetType != 1)) {
    cerr << "SCorrelatorJetTree::AddClusters - Warning - trying to add calorimeter clusters to charged jets!" << endl;
  }

  // get vertex map
  GlobalVertexMap *vertexmap = findNode::getClass<GlobalVertexMap>(topNode, "GlobalVertexMap");
  if (!vertexmap) {
    cerr << "SCorrelatorJetTree::getEmcalClusters - Fatal Error - GlobalVertexMap node is missing!\n"
         << "  Please turn on the do_global flag in the main macro in order to reconstruct the global vertex!"
         << endl;
    assert(vertexmap);
    return;
  }
  if (vertexmap -> empty()) {
    cerr << "SCorrelatorJetTree::getEmcalClusters - Fatal Error - GlobalVertexMap node is empty!\n"
         << "  Please turn on the do_global flag in the main macro in order to reconstruct the global vertex!"
         << endl;
    return;
  }

  // grab vertex
  GlobalVertex *vtx = vertexmap -> begin() -> second;
  if (vtx == nullptr) return;

  // add emcal clusters if needed
  unsigned int iPart     = particles.size();
  unsigned int nClustTot = 0;
  unsigned int nClustAcc = 0;
  unsigned int nClustEM  = 0;
  unsigned int nClustH   = 0;
  double       eClustSum = 0.;
  if (m_add_EMCal_clusters) {

    // grab em cluster containter
    RawClusterContainer *clustersEMC = findNode::getClass<RawClusterContainer>(topNode, "CLUSTER_CEMC");
    if (!clustersEMC) {
      cout << PHWHERE
           << "PANIC: EMCal cluster node is missing, can't collect EMCal clusters!"
           << endl;
      return;
    }

    // loop over em clusters
    RawClusterContainer::ConstRange    begin_end_EMC = clustersEMC -> getClusters();
    RawClusterContainer::ConstIterator clusIter_EMC;
    for (clusIter_EMC = begin_end_EMC.first; clusIter_EMC != begin_end_EMC.second; ++clusIter_EMC) {

      // grab cluster
      const RawCluster *cluster = clusIter_EMC -> second;
      if (!cluster) {
        continue;
      } else {
        ++nClustTot;
      }

      // construct vertex and get 4-momentum
      const double vX = vtx -> get_x();
      const double vY = vtx -> get_y();
      const double vZ = vtx -> get_z();

      CLHEP::Hep3Vector vertex(vX, vY, vZ);
      CLHEP::Hep3Vector E_vec_cluster = RawClusterUtility::GetECoreVec(*cluster, vertex);

      // check if good
      const bool isGoodClust = IsGoodEMCalCluster(E_vec_cluster);
      if (!isGoodClust) {
        continue;
      } else {
        ++nClustAcc;
      }

      // create pseudojet and add to constituent vector
      const int    emClustID  = cluster -> get_id();
      const double emClustE   = E_vec_cluster.mag();
      const double emClustPt  = E_vec_cluster.perp();
      const double emClustPhi = E_vec_cluster.getPhi();
      const double emClustPx  = emClustPt * cos(emClustPhi);
      const double emClustPy  = emClustPt * sin(emClustPhi);
      const double emClustPz  = sqrt((emClustE * emClustE) - (emClustPx * emClustPx) - (emClustPy * emClustPy));

      fastjet::PseudoJet fjCluster(emClustPx, emClustPy, emClustPz, emClustE);
      fjCluster.set_user_index(iPart);
      particles.push_back(fjCluster);

      // add em cluster to fastjet map
      pair<int, pair<Jet::SRC, int>> jetEMClustPair(iPart, make_pair(Jet::SRC::CEMC_CLUSTER, emClustID));
      fjMap.insert(jetEMClustPair);

      // fill QA histograms, increment sums and counters
      m_hObjectQA[OBJECT::ECLUST][INFO::PT]  -> Fill(fjCluster.perp());
      m_hObjectQA[OBJECT::ECLUST][INFO::ETA] -> Fill(fjCluster.pseudorapidity());
      m_hObjectQA[OBJECT::ECLUST][INFO::PHI] -> Fill(fjCluster.phi_std());
      m_hObjectQA[OBJECT::ECLUST][INFO::ENE] -> Fill(fjCluster.E());
      eClustSum += emClustE;
      ++nClustEM;
      ++iPart;
    }  // end em cluster loop
  }  // end if (m_add_EMCal_clusters)

  //  add hcal clusters if needed
  if (m_add_HCal_clusters) {

    // grab ih cluster container
    RawClusterContainer *clustersHCALIN = findNode::getClass<RawClusterContainer>(topNode, "CLUSTER_HCALIN");
    if (!clustersHCALIN) {
      cerr << PHWHERE
           << "PANIC: Inner HCal cluster node is missing, can't collect inner HCal clusters!"
           << endl;
      return;
    }

    // Loop over ih clusters
    RawClusterContainer::ConstRange    begin_end_HCALIN = clustersHCALIN -> getClusters();
    RawClusterContainer::ConstIterator clusIter_HCALIN;
    for (clusIter_HCALIN = begin_end_HCALIN.first; clusIter_HCALIN != begin_end_HCALIN.second; ++clusIter_HCALIN) {

      // get ih cluster
      const RawCluster *cluster = clusIter_HCALIN -> second;
      if (!cluster) {
        continue;
      } else {
        ++nClustTot;
      }

      // construct vertex and get 4-momentum
      const double vX = vtx -> get_x();
      const double vY = vtx -> get_y();
      const double vZ = vtx -> get_z();

      CLHEP::Hep3Vector vertex(vX, vY, vZ);
      CLHEP::Hep3Vector E_vec_cluster = RawClusterUtility::GetECoreVec(*cluster, vertex);

      // check if good
      const bool isGoodClust = IsGoodHCalCluster(E_vec_cluster);
      if (!isGoodClust) {
        continue;
      } else {
        ++nClustAcc;
      }

      // create pseudojet and add to constituent vector
      const int    ihClustID  = cluster -> get_id();
      const double ihClustE   = E_vec_cluster.mag();
      const double ihClustPt  = E_vec_cluster.perp();
      const double ihClustPhi = E_vec_cluster.getPhi();
      const double ihClustPx  = ihClustPt * cos(ihClustPhi);
      const double ihClustPy  = ihClustPt * sin(ihClustPhi);
      const double ihClustPz  = sqrt((ihClustE * ihClustE) - (ihClustPx * ihClustPx) - (ihClustPy * ihClustPy));

      fastjet::PseudoJet fjCluster(ihClustPx, ihClustPy, ihClustPz, ihClustE);
      fjCluster.set_user_index(iPart);
      particles.push_back(fjCluster);

      // add ih cluster to fastjet map
      pair<int, pair<Jet::SRC, int>> jetIHClustPair(iPart, make_pair(Jet::SRC::HCALIN_CLUSTER, ihClustID));
      fjMap.insert(jetIHClustPair);

      // fill QA histograms, increment sums and counters
      m_hObjectQA[OBJECT::HCLUST][INFO::PT]  -> Fill(fjCluster.perp());
      m_hObjectQA[OBJECT::HCLUST][INFO::ETA] -> Fill(fjCluster.pseudorapidity());
      m_hObjectQA[OBJECT::HCLUST][INFO::PHI] -> Fill(fjCluster.phi_std());
      m_hObjectQA[OBJECT::HCLUST][INFO::ENE] -> Fill(fjCluster.E());
      eClustSum += ihClustE;
      ++nClustH;
      ++iPart;
    }  // end ih cluster loop

    // grab oh cluster container
    RawClusterContainer *clustersHCALOUT = findNode::getClass<RawClusterContainer>(topNode, "CLUSTER_HCALOUT");
    if (!clustersHCALOUT) {
      cerr << PHWHERE
           << "PANIC: Outer HCal cluster node is missing, can't collect outer HCal clusters!"
           << endl;
      return;
    }

    // loop over oh clusters
    RawClusterContainer::ConstRange    begin_end_HCALOUT = clustersHCALOUT -> getClusters();
    RawClusterContainer::ConstIterator clusIter_HCALOUT;
    for (clusIter_HCALOUT = begin_end_HCALOUT.first; clusIter_HCALOUT != begin_end_HCALOUT.second; ++clusIter_HCALOUT) {

      // get oh cluster
      const RawCluster *cluster = clusIter_HCALOUT -> second;
      if (!cluster) {
        continue;
      } else {
        ++nClustTot;
      }

      // construct vertex and get 4-momentum
      const double vX = vtx -> get_x();
      const double vY = vtx -> get_y();
      const double vZ = vtx -> get_z();

      CLHEP::Hep3Vector vertex(vX, vY, vZ);
      CLHEP::Hep3Vector E_vec_cluster = RawClusterUtility::GetECoreVec(*cluster, vertex);

      // check if good
      const bool isGoodClust = IsGoodHCalCluster(E_vec_cluster);
      if (!isGoodClust) {
        continue;
      } else {
        ++nClustAcc;
      }

      // create pseudojet and add to constituent vector
      const int    ohClustID  = cluster -> get_id();
      const double ohClustE   = E_vec_cluster.mag();
      const double ohClustPt  = E_vec_cluster.perp();
      const double ohClustPhi = E_vec_cluster.getPhi();
      const double ohClustPx  = ohClustPt * cos(ohClustPhi);
      const double ohClustPy  = ohClustPt * sin(ohClustPhi);
      const double ohClustPz  = sqrt((ohClustE * ohClustE) - (ohClustPx * ohClustPx) - (ohClustPy * ohClustPy));

      fastjet::PseudoJet fjCluster(ohClustPx, ohClustPy, ohClustPz, ohClustE);
      fjCluster.set_user_index(iPart);
      particles.push_back(fjCluster);

      // add oh cluster to fastjet map
      pair<int, pair<Jet::SRC, int>> jetOHClustPair(iPart, make_pair(Jet::SRC::HCALOUT_CLUSTER, ohClustID));
      fjMap.insert(jetOHClustPair);

      // fill QA histograms, increment sums and counters
      m_hObjectQA[OBJECT::HCLUST][INFO::PT]  -> Fill(fjCluster.perp());
      m_hObjectQA[OBJECT::HCLUST][INFO::ETA] -> Fill(fjCluster.pseudorapidity());
      m_hObjectQA[OBJECT::HCLUST][INFO::PHI] -> Fill(fjCluster.phi_std());
      m_hObjectQA[OBJECT::HCLUST][INFO::ENE] -> Fill(fjCluster.E());
      eClustSum += ohClustE;
      ++nClustH;
      ++iPart;
    }  // end oh cluster loop
  }  // end if (m_add_HCal_clusters)

  // fill QA histograms
  m_hNumObject[OBJECT::ECLUST]           -> Fill(nClustEM);  // TODO: fill this one in cst collection method
  m_hNumObject[OBJECT::HCLUST]           -> Fill(nClustH);   // TODO: fill this one in cst collection method
  m_hNumCstAccept[CST_TYPE::CALO_CST][0] -> Fill(nClustTot);
  m_hNumCstAccept[CST_TYPE::CALO_CST][1] -> Fill(nClustAcc);  // TODO: fill this one in jet finding method
  m_hSumCstEne[CST_TYPE::CALO_CST]       -> Fill(eClustSum);
  return;

}  // end 'AddClusters(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&)'



void SCorrelatorJetTree::AddParticles(PHCompositeNode *topNode, vector<PseudoJet> &particles, map<int, pair<Jet::SRC, int>> &fjMap) {

  // print debug statement
  if (m_doDebug) {
    cout << "SCorrelatorJetTree::AddParticles(PHComposite*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&) Adding MC particles..." << endl;
  }

  // grab mc event map
  PHHepMCGenEventMap *hepmceventmap = findNode::getClass<PHHepMCGenEventMap>(topNode, "PHHepMCGenEventMap");
  if (!hepmceventmap) {
    cerr << PHWHERE
         << "PANIC: HEPMC event map node is missing, can't collect HEPMC truth particles!"
         << endl;
    return;
  }

  // grab mc event & check if good
  PHHepMCGenEvent *hepmcevent = hepmceventmap -> get(1);
  if (!hepmcevent) {
    cerr << PHWHERE
         << "PANIC: Couldn't grab HepMCEvent begin()! Abandoning particle collection!"
         << endl;
    return;
  }

  HepMC::GenEvent *hepMCevent = hepmcevent -> getEvent();
  if (!hepMCevent) {
    cerr << PHWHERE
         << "PANIC: Couldn't grab HepMC event! Abandoning particle collection!"
         << endl;
    return;
  }

  // loop over particles
  unsigned int iPart   = particles.size();
  unsigned int nParTot = 0;
  unsigned int nParAcc = 0;
  double       eParSum = 0.;
  for (HepMC::GenEvent::particle_const_iterator p = hepMCevent -> particles_begin(); p != hepMCevent -> particles_end(); ++p) {

    // check if particle is final state
    const bool isFinalState = ((*p) -> status() == 1);
    if (!isFinalState) {
      continue;
    } else {
      ++nParTot;
    }

    // check if particle is good
    const bool isGoodPar = IsGoodParticle(*p);
    if (!isGoodPar) {
      continue;
    } else {
      ++nParAcc;
    }

    // create pseudojet & add to constituent vector
    const int    parID = (*p) -> barcode();
    const double parPx = (*p) -> momentum().px();
    const double parPy = (*p) -> momentum().py();
    const double parPz = (*p) -> momentum().pz();
    const double parE  = (*p) -> momentum().e();

    fastjet::PseudoJet fjMCParticle(parPx, parPy, parPz, parE);
    fjMCParticle.set_user_index(iPart);
    particles.push_back(fjMCParticle);

    // add particle to mc fastjet map
    pair<int, pair<Jet::SRC, int>> jetPartPair(iPart, make_pair(Jet::SRC::PARTICLE, parID));
    fjMap.insert(jetPartPair);

    // fill QA histograms, increment sums and counters
    m_hObjectQA[OBJECT::PART][INFO::PT]  -> Fill(fjMCParticle.perp());
    m_hObjectQA[OBJECT::PART][INFO::ETA] -> Fill(fjMCParticle.pseudorapidity());
    m_hObjectQA[OBJECT::PART][INFO::PHI] -> Fill(fjMCParticle.phi_std());
    m_hObjectQA[OBJECT::PART][INFO::ENE] -> Fill(fjMCParticle.E());
    eParSum += parE;
    ++iPart;
  }  // end particle loop

  // fill QA histograms
  m_hNumObject[OBJECT::PART]             -> Fill(nParAcc);  // TODO: fill this one in cst collection method
  m_hNumCstAccept[CST_TYPE::PART_CST][0] -> Fill(nParTot);
  m_hNumCstAccept[CST_TYPE::PART_CST][1] -> Fill(nParAcc);  // TODO: fill this one in jet finding method
  m_hSumCstEne[CST_TYPE::PART_CST]       -> Fill(eParSum);
  return;

}  // end 'AddParticles(PHCompositeNode*, vector<PseudoJet>&, map<int, pair<Jet::SRC, int>>&)'

// end ------------------------------------------------------------------------
