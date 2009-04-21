/* Copyright (C) 2005  Christoph Helma <helma@in-silico.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef PREDICTOR_H
#define PREDICTOR_H

#include "boost/smart_ptr.hpp"
#include "feature-generation.h"
#include "activity-db.h"
/*#include "fminer.h"*/
#include "model.h"
#include "time.h"

using namespace std;
using namespace OpenBabel;
using namespace boost;

extern bool kernel;
extern bool quantitative;

//! make predictions from training data (structures, activities, features)
template <class MolType, class FeatureType, class ActivityType>
class Predictor {

public:

    typedef FeatMol < MolType, FeatureType, ActivityType > * MolRef ;
    typedef shared_ptr<FeatMol < MolType, FeatureType, ActivityType > > sMolRef ;
    typedef shared_ptr<FeatMol < OBLazMol, ClassFeat, bool> > sClassMolRef;
    typedef Feature<OBLinFrag> * OBLinFragRef;
    typedef Feature<FeatureType> * FeatRef;

private:

    //! feature generation for new structures
    shared_ptr<FeatGen <MolType, FeatureType, ActivityType> > feat_gen;
    //! training structures
    shared_ptr<ActMolVect <MolType, FeatureType, ActivityType> > train_structures;
    //! test structures for batch predictions
    shared_ptr<MolVect <MolType, FeatureType, ActivityType> > test_structures;
    //! model
    shared_ptr<MetaModel<MolType, FeatureType, ActivityType> > model;


    //! neighbors for the prediction of the current query structure
    vector<sMolRef> neighbors;
    //! input file with activities
    char* a_file;
    //! make leave-one-out crossvalidation?
    bool loo;
		//! output object
		shared_ptr<Out> out;

public:

    //! Predictor constructor for LOO
    Predictor(char * structure_file, char * act_file, char * feat_file, shared_ptr<Out> out): a_file(NULL), loo(false), out(out) {
        train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
        if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out) );
        else model.reset( new Model<MolType, FeatureType, ActivityType>(out) );
    };

    //! Predictor constructor for single SMILES prediction
    Predictor(char * structure_file, char * act_file, char * feat_file, char * alphabet_file, shared_ptr<Out> out): a_file(alphabet_file), loo(false), out(out){
        train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
        if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out ));
        else model.reset(new Model<MolType, FeatureType, ActivityType>(out));
    }

    //! Predictor constructor for batch prediction
    Predictor(char * structure_file, char * act_file, char * feat_file, char * alphabet_file, char * input_file, shared_ptr<Out> out): a_file(alphabet_file), loo(false), out(out){
        train_structures.reset( new ActMolVect <MolType, FeatureType, ActivityType>(act_file, feat_file, structure_file, out) );
        test_structures.reset( new MolVect <MolType, FeatureType, ActivityType>(input_file, out) );
        if (kernel) model.reset( new KernelModel<MolType, FeatureType, ActivityType>(out) );
        else model.reset( new Model<MolType, FeatureType, ActivityType>(out) );
    }


    //! predict a single smiles
    void predict_smi(string smiles);

    //! batch prediction: predict witheld fold, i.e. compounds must occur in smi database, do "make testset" to generate fold tool.
    void predict_fold();

    //! batch predictions: predict arbitrary comps.
    void predict_file();

    //! leave one out crossvalidation
    void loo_predict();

    //! predict a test structure
    void predict(sMolRef test_compound, bool recalculate, bool verbose);

    //! predict the activity act for the query structure
    void knn_predict(sMolRef test, string act, bool verbose);

    void print_neighbors(string act);

    //! set the output object (e.g. switch between console and socket)
    void set_output(shared_ptr<Out> newout);

    string get_yaml();

    //! match features (SMARTS) from a file
    void match_file_smarts(char * file);

    //! apply y-scrambling (aka response permutation testing, see Eriksson et al. 2003)
    vector<map<string, vector<ActivityType> > > y_scrambling();

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_fold() {

    typename vector<sMolRef>::iterator cur_dup;
    typedef shared_ptr<Feature<FeatureType> > sFeatRef;
    //Fminer* fminer = NULL; 

    sMolRef cur_mol;
    int test_size = test_structures->get_size();
 
    vector<sFeatRef>* features = train_structures->get_features();
    typename vector<sFeatRef>::iterator feat_it;

    map<string, vector<string> > feat_map;


    // ADD FRAGMENTS FROM THE TRAINING SET TO TEST SET STRUCTURES
    for (int n = 0; n < test_size; n++) {
        cur_mol = test_structures->get_compound(n);
     
        /*   
        delete fminer;
        fminer = new Fminer();

        fminer->SetChisqActive(false); // Disable activity values
        fminer->SetConsoleOut(false);

        // Get all fragments
        fminer->SetMinfreq(1);
        fminer->SetRefineSingles(true); 

        // Get most specialized pattern of each BBRC
        fminer->SetMostSpecTreesOnly(true); 

        fminer->AddCompound(cur_mol->get_smiles(), atoi(cur_mol->get_id().c_str()));

        cout << "Mining " << cur_mol->get_smiles() << endl;
        for (int x = 0; x < (int) fminer->GetNoRootNodes(); x++ ) {
            vector<string>* result = fminer->MineRoot(x);
            for(unsigned int y = 0; y < result->size(); y++) {
                cout << (*result)[y] << endl;
            }
        }
        cout << endl;

        */


        for (feat_it=features->begin(); feat_it!=features->end(); feat_it++) {
            shared_ptr<OBSmartsPattern> frag (new OBSmartsPattern() );
            if (!frag->Init((*feat_it)->get_name())) {
                cerr << "Warning! predict_fold(): OBSmartsFrag '" << (*feat_it)->get_name() << "' failed to initialize!" << endl;
            }
            else {
                if ( frag->Match((*(cur_mol->get_mol_ref())),true) ) {
                     cur_mol->add_feature((*feat_it).get());
                     feat_map[(*feat_it)->get_name()].push_back(cur_mol->get_id());
                }
            }
        }
	
        // REMOVE ALL TEST STRUCTURES
        vector<sMolRef> duplicates = train_structures->remove_duplicates(cur_mol);
        if (duplicates.size()) {
            *out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " removed from the training set!\n";
            out->print_err();
        }

    }
    //delete fminer;

    for (feat_it=features->begin(); feat_it!=features->end(); feat_it++) {
         if (feat_map[(*feat_it)->get_name()].size() == 0)
	    feat_map[(*feat_it)->get_name()].push_back("");
    }

    // CAN USE BELOW BLOCK TO OUTPUT TEST FRAGNENTS
    /*
    typename map<string, vector<string> >::iterator feat_map_it;
    for (feat_map_it = feat_map.begin(); feat_map_it != feat_map.end(); feat_map_it++) {
	    cout << feat_map_it->first << "\t[ ";
	    typename vector<string>::iterator c_it;
	    for (c_it = feat_map_it->second.begin(); c_it != feat_map_it->second.end(); c_it++) {
		    cout << (*c_it)  << " ";
	    }
	    cout << "]" << endl;
    }
    */

    // PREDICT FOLD
    for (int n = 0; n < test_size; n++) {
        cur_mol = test_structures->get_compound(n);

        *out << "Predicting external test id " << cur_mol->get_id() << endl;
        out->print_err();

        // recalculate frequencies and and significance only for the first time
        if (n == 0)
            this->predict(cur_mol, true);
        else
            this->predict(cur_mol, false);

    }

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_file() {

    typename vector<sMolRef>::iterator cur_dup;

    sMolRef cur_mol;
    int test_size = test_structures->get_size();

    for (int n = 0; n < test_size; n++) {

        cur_mol = test_structures->get_compound(n);
        feat_gen.reset(new FeatGen <MolType, FeatureType, ActivityType>(a_file, train_structures, cur_mol,out));
        feat_gen->generate_linfrag(train_structures,cur_mol);

        //cur_mol->print();

        // check if the compound is already in the database

        *out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
        out->print_err();

        vector<sMolRef> duplicates = train_structures->remove_duplicates(cur_mol);

        if (duplicates.size() > 1) {
            *out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
            out->print_err();
        }

        // recalculate frequencies and and significance only if necessary

        if (n == 0)
            this->predict(cur_mol, true, true);
        else if (duplicates.size() > 0) {
            this->predict(cur_mol, true, true);
        }
        else
            this->predict(cur_mol, false, true);

        // restore duplicates for batch predictions

        if (duplicates.size() >= 1) {
            for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
                (*cur_dup)->restore();
            }
        }
    }

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::loo_predict() {

    loo = true;
    sMolRef cur_mol;
    typename vector<sMolRef>::iterator cur_dup;


    clock_t t1 = clock();
    cerr << "Precomputing significance values... ";

    if (!quantitative) {
        // MG : precompute
        vector<ActivityType> activity_values;
        vector<string> activity_names = train_structures->get_activity_names();
        typename vector<string>::iterator cur_act;

        for (cur_act = activity_names.begin(); cur_act != activity_names.end(); cur_act++) {

            activity_values = train_structures->get_activity_values(*cur_act);
            train_structures->precompute_feature_significance(*cur_act, activity_values);

        }
        // MG
    }

    clock_t t2 = clock();
    cerr << "done (" << (float)(t2-t1)/CLOCKS_PER_SEC << "sec)!" << endl;

    for (int n = 0; n < train_structures->get_size(); n++) {

        t1 = clock();

        cur_mol = train_structures->get_compound(n);

        // make query compound unavailable as train structure in this round
        *out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
        out->print_err();
        vector<sMolRef> duplicates = train_structures->remove_duplicates(cur_mol);
        if (duplicates.size() > 1) {
            *out << duplicates.size() << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
            out->print_err();
        }
        // predict by recalculating significance values
        this->predict(cur_mol,true,false);

        // recover query compound as train structure for the next round
        if (duplicates.size() >= 1) {
            for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
                (*cur_dup)->restore();
            }
        }

        t2 = clock();
        static float avg_s = 0;
        float s = (float)(t2-t1)/CLOCKS_PER_SEC;
        avg_s = (avg_s * n + s) / float(n+1);

        cerr << "Prediction " << n+1 << "/" << train_structures->get_size() << " of structure " <<cur_mol->get_id() <<
        " took " << s << " sec (avg is " << avg_s << " sec)" << endl;
    }


};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict_smi(string smiles) {

    bool recalculate = true;
    vector<sMolRef> duplicates ;
    typename vector<sMolRef>::iterator cur_dup;

    shared_ptr<FeatMol <MolType, FeatureType, ActivityType> > cur_mol ( new FeatMol<MolType, FeatureType, ActivityType>(0,"test structure",smiles,out) );

    *out << "Looking for " << cur_mol->get_smiles() << " in the training set\n";
    out->print_err();
    duplicates = train_structures->remove_duplicates(cur_mol);

    //delete feat_gen;
    feat_gen.reset( new FeatGen <MolType, FeatureType, ActivityType>(a_file, train_structures, cur_mol,out)) ;
    feat_gen->generate_linfrag(train_structures,cur_mol);

    if (duplicates.size() > 1) {
        *out << int(duplicates.size()) << " instances of " << cur_mol->get_smiles() << " in the training set!\n";
        out->print_err();
    }
    else if (duplicates.size() > 0) {
        this->predict(cur_mol, true, true);
    }
    else if (recalculate) {
        this->predict(cur_mol, true, true);
    }
    else
        this->predict(cur_mol, false, true);

    // restore duplicates for batch predictions

    if (duplicates.size() >= 1) {
        for (cur_dup=duplicates.begin(); cur_dup != duplicates.end(); cur_dup++) {
            (*cur_dup)->restore();
        }
    }

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::predict(sMolRef test, bool recalculate, bool verbose=true) {

    vector<ActivityType> activity_values;
    vector<string> activity_names = train_structures->get_activity_names();
    typename vector<string>::iterator cur_act;

    // determine common features in the training set
    train_structures->common_features(test);

    for (cur_act = activity_names.begin(); cur_act != activity_names.end(); cur_act++) {


        if (!loo || test->db_act_available(*cur_act)) {	// make loo predictions only for activities with measured values

            *out << "---\n";
            out->print();

            if (recalculate) {

                if (!loo || quantitative) {
                    activity_values = train_structures->get_activity_values(*cur_act);
                    train_structures->feature_significance(*cur_act, activity_values);	// AM: feature significance
                }

                // MG
                else {
                    typename vector<FeatRef>::iterator cur_feat;
                    vector<ActivityType> tmp_activities;

                    tmp_activities = test->get_act(*cur_act);
                    if (tmp_activities.size() > 1) {
                        fprintf(stderr, "Current test structure has more than one activity value");
                        exit(1);
                    }
                    ClassFeat::set_cur_str_active( *tmp_activities.begin() );

                    // label features that occur in current test structure
                    vector<FeatRef> test_features = test->get_features();
                    for (cur_feat=test_features.begin(); cur_feat!=test_features.end(); cur_feat++){
                        (*cur_feat)->set_cur_feat_occurs( true );
                    }
                }
            }
            else {
                *out << "Significances for " << *cur_act << " not recalculated.\n";
                out->print_err();
            }

            train_structures->relevant_features(test, *cur_act);
            this->knn_predict(test,*cur_act);
            *out << "\n";
            out->print();


            if (loo && !quantitative) {
                // MG: remove label that feature occurs in current test structure
                typename vector<FeatRef>::iterator cur_feat;
                vector<FeatRef> test_features = test->get_features();
                for (cur_feat=test_features.begin(); cur_feat!=test_features.end(); cur_feat++){
                    (*cur_feat)->set_cur_feat_occurs( false );
                }
                // MG
            }
        }

        else cerr << "Database activity not available." << endl;
    }

};


template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::knn_predict(sMolRef test, string act, bool verbose=true)  {

    // determine neighbors
    train_structures->get_neighbors(act, &neighbors);

    // determine and print
    train_structures->determine_unknown(act, test);

    // calculate and print predicition
    test->print();
    test->print_db_activity(act,loo);
    model->calculate_prediction(test, &neighbors, act);
    *out << "endpoint: '" << act << "'\n";
    out->print();

    // print neighbors
    if (verbose) {
        *out << "neighbors:\n";
        this->print_neighbors(act);
        *out << "features:\n";
        test->print_features(act);
        out->print();
    }
    *out << "unknown_features:\n";
    test->print_unknown(act);
    out->print();
    test->delete_unknown();

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::print_neighbors(string act) {

    int n;
    typename vector<sMolRef>::iterator cur_n;

    sort(neighbors.begin(),neighbors.end(),greater_sim<MolType,FeatureType,ActivityType>());

    if (neighbors.size()>0) {

        n = 0;
        for (cur_n = neighbors.begin(); (cur_n != neighbors.end()); cur_n++) {
            (*cur_n)->print_neighbor(act);
            n++;
        }
    }

};

template <class MolType, class FeatureType, class ActivityType>
void Predictor<MolType, FeatureType, ActivityType>::set_output(shared_ptr<Out> newout) {

    out = newout ;
    int train_size = train_structures->get_size();
    model->set_output(out);

    for (int n = 0; n < train_size; n++) {
        train_structures->get_compound(n)->set_output(out);
    }

};

template <class MolType, class FeatureType, class ActivityType>
string Predictor<MolType, FeatureType, ActivityType>::get_yaml() {
  return this->out->get_yaml();
}

/*
template <class MolType, class FeatureType, class ActivityType>
vector<map<string, vector<ActivityType> > > Predictor<MolType, FeatureType, ActivityType>::y_scrambling() {

	map<string, vector<ActivityType> > val;
	typename vector<map<string, vector<ActivityType> > >::iterator act_it;
	vector<MolRef> tc;
	typename vector<MolRef>::iterator tc_it;

	// gather activities
	vector<map<string, vector<ActivityType> > > act_avail;
	vector<map<string, vector<ActivityType> > > act_ori;

	tc=train_structures->get_compounds();
	for (tc_it = tc.begin(); tc_it != tc.end(); tc_it++) {
		act_avail.push_back((*tc_it)->get_activities());
	}
	act_ori.clear();
	act_ori.assign(act_avail.begin(), act_avail.end());

	// draw random activity without replacement
	for (int n=0; n<train_structures->get_size(); n++) {
		srand(static_cast<unsigned int>(clock()));
		double dpos = double(rand()) / (double(RAND_MAX) + 1.0);
		int ipos = (int) (dpos*act_avail.size());
		val.clear();
		val = act_avail.at(ipos);
		for (act_it = act_avail.begin(); act_it != act_avail.end(); act_it++) {
			if (*(act_it) == val) break;
		}

		act_avail.erase(act_it);

		train_structures->get_compound(n)->replace_activities(val);
	}

	return act_ori;

}
*/

#endif
