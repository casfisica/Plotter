#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH1.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include <string>
#include <iostream>
#include "TTreeReader.h"
#include <tuple>
#include <vector>
#include <utility>      // std::pair, std::make_pair
//#include "../test/Event.h"


/*
Inspired by
https://root-forum.cern.ch/t/listing-contents-of-a-directory/1046
*/
class list_files{
    private:
    // Vector to hold file names
    std::vector<string> filelist = std::vector<string>();
    //std::vector<string> filelist;
    //const char *_dirname;
   
    
    public:
    /*Constructor*/
    //list_files(const char *_dirname):_dirname(dirname){}
    list_files(void){}
    /*Destructor*/
    ~list_files(){}
    
    void collectAllFiles(const char *_dirname="/", const char *ext=".root")
    {
        TSystemDirectory dir(_dirname, _dirname); 
        TList *files = dir.GetListOfFiles(); 
        std::string dirnamestring= (std::string) _dirname;

        if (files) { 
            TSystemFile *file; 
            TString fname; 
            TIter next(files); 
            while ((file=(TSystemFile*)next())) { 
                fname = file->GetName(); 
                if(!file->IsDirectory() && fname.EndsWith(ext)) { 
                    //cout << fname.Data() << endl; 
                    filelist.push_back(dirnamestring+"/"+fname.Data());
                }
            
                // If this which we found now, is a directory, recursively 
                // call the function again
                if (file->IsDirectory() && strcmp(fname,".") != 0 && strcmp(fname,"..")) { 
                    std::string tempdirnamestring=dirnamestring+"/"+fname.Data();
                    collectAllFiles(tempdirnamestring.c_str(),ext);
                    //collectAllFiles(tempdirnamestring.c_str(),ext,filelist);
                    //std::cout<<fname.Data()<<std::endl;
                }      
            } 
        }
    }

    /*Get Method*/
    std::vector<string> GetFileList(const char *_dirname="/", const char *ext=".root"){
        collectAllFiles(_dirname,ext);
        return filelist;
    }
};

/********************************************************
Use
*********************************************
list_files lista;
a=lista.GetFileList(dirname.c_str(),ext.c_str());
a.size()
a.at(290)
*********************************************************/


int CreateRandomTree(std::string name){
    /*
    Create trees for tries   
    */
    gROOT->Reset();
    /*
    Create Ttrees for tries 
    */
    std::string Name = name+".root";
    //convierto en cadena de caracteres constante (es lo que recibe la función)
    const char * constName = Name.c_str();
    //const char * constname = name.c_str();
    TFile hfile(constName,"RECREATE");
    //The Ttrees must to have the same name "tree" in this case, "Delphes"
    TTree *T = new TTree("tree","test Ttree");
    Double_t px,py,pz;
    UShort_t i;
    T->Branch("px",&px,"px/D");
    T->Branch("py",&py,"py/D");
    T->Branch("pz",&pz,"pz/D");
    T->Branch("i",&i,"i/s");
    for (i = 0; i < 65000; i++) {
        px = (Double_t) gRandom->Binomial(500,0.05);
        pz = (Double_t) gRandom->Landau(-10,100);
        py = (Double_t) gRandom->Gaus(0,1000);
        T->Fill();
    }

    //T->Print();
    hfile.Write();
    hfile.Close();
   return 0;
}

void* TypeAllocate(std::string str){
    /*
    C : a character string terminated by the 0 character
    B : an 8 bit signed integer (Char_t)
    b : an 8 bit unsigned integer (UChar_t)
    S : a 16 bit signed integer (Short_t)
    s : a 16 bit unsigned integer (UShort_t)
    I : a 32 bit signed integer (Int_t)
    i : a 32 bit unsigned integer (UInt_t)
    F : a 32 bit floating point (Float_t)
    D : a 64 bit floating point (Double_t)
    L : a 64 bit signed integer (Long64_t)
    l : a 64 bit unsigned integer (ULong64_t)
    O : [the letter o, not a zero] a boolean (Bool_t)
    */ 
    if (str=="C") {
        char *ptr=new char;
        return (void*)ptr;
    }
    if (str=="B"){
        Char_t *ptr=new Char_t;
        return (void*)ptr;
    }
    if (str=="b"){
        UChar_t *ptr=new UChar_t;
        return (void*)ptr;
    }
    if (str=="S"){
        Short_t *ptr=new Short_t;
        return (void*)ptr;
    }
    if (str=="s"){
        UShort_t *ptr=new UShort_t;
        return (void*)ptr;
    }
    if (str=="I"){
        Int_t *ptr=new Int_t;
        return (void*)ptr;
    }
    if (str=="i"){
        UInt_t *ptr=new UInt_t;
        return (void*)ptr;
    }
    if (str=="F"){
        Float_t *ptr=new Float_t;
        return (void*)ptr;
    }
    if (str=="D"){
        Double_t *ptr=new Double_t;
        return (void*)ptr;
    }
    if (str=="L"){
        Long64_t *ptr=new Long64_t;
        return (void*)ptr;
    }
    if (str=="l"){
        ULong64_t *ptr=new ULong64_t;
        return (void*)ptr;
    }
    if (str=="O"){
        Bool_t *ptr=new Bool_t;
        return (void*)ptr;
    }
    return NULL;
}

std::map<std::string,TH1D *> Plotter(TChain * chain1, Bool_t Debug=0, std::vector<std::string> Branches={},Double_t Max=100.0, Double_t Min=0.0, Int_t NBines=100)
{
    std::map<std::string,TChain*> ChainMap;
    std::map<std::string,void*> ptrMap;
    std::map<std::string,TH1D *> HistoMap;
    std::vector<std::pair <std::string,std::string>> NamesArray;
    
    auto Array=chain1->GetListOfBranches();
    Int_t ArrayEntries=Array->GetEntries();
    for ( Int_t j=0 ; j<ArrayEntries ; j++ ){
        auto object = Array->At(j);
        std::string name = object->GetName();
        std::string nameplus = name+chain1->GetTitle();

        if(Debug){
            object->Print();
            std:cout<<"Name: "<<nameplus<<std::endl;
        }
        //See if name is in the list of the user
        if(!Branches.empty()){
            Bool_t flag=0;
            for (auto it: Branches){
                if(it==name)  flag=1;
            }
            if(!flag) continue;
        }
        //Getting the type of the branch
        std::string Title=object->GetTitle();
        std::size_t found=Title.find_last_of('/');
        if (found!=std::string::npos){
            std::string Type = Title.substr(found+1);
            //   Specify address where to read the event object
            if(Debug) std::cout<<"Type: "<<Type<<std::endl;
            //Aloco un puntero al tipo de dato del branch, y lo guardo en el map, con el key nombre
            ptrMap[name]=TypeAllocate(Type);
            if(Debug) std::cout<<"ptr: "<<ptrMap[name]<<std::endl;
            //El branch se va a leer de la dirección del puntero gurdado en el PointMap[name]
            chain1->SetBranchAddress(name.c_str(), ptrMap[name]);
            //   Create an histogram
            HistoMap[nameplus] = new TH1D(nameplus.c_str(),Title.c_str(),NBines,Min,Max);
            //Saving the names
            NamesArray.push_back(std::make_pair(name,nameplus));

        }
        else{
            std::cout<<"Error no type defined in "<<Title<<std::endl;
        }
    }
    
    
    //Filling the Histograms
    for (Int_t j=0;j<chain1->GetEntries();j++) {
        chain1->GetEvent(j);              //read complete accepted event in memory
        if(Debug) if(j==10) break;
        for (auto it:NamesArray){            
            HistoMap[it.second]->Fill(*((Double_t*)ptrMap[it.first]) /*, el peso va acá*/);  //Fill histogram with 
            if(Debug) {
                std::cout<<"Name: "<<it.second<<std::endl;
                std::cout<<"Pointer: "<<ptrMap[it.first]<<std::endl;
            }
        }                            //iterar sobre Sobre los branches del mapa              
    }
    
 return HistoMap;
}


int LibPlotter(void){
return 0;
}