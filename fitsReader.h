
#ifndef __fitsReader_h
#define __fitsReader_h

#include "fitsio.h"
#include "splotch/splotchutils.h"

class fitsReader {
public:
    fitsReader();
    ~fitsReader();

    int m_type;
    float rms_koef;

    struct VALS {
        int ii;
        float val;
        VALS(int i, float v)
        {
            ii = i;
            val = v;
        };
    };

    void SetFileName(std::string name);
    std::string GetFileName() { return filename; }

    double getInitSlice() { return initSlice; }

    bool is3D;
    void ReadFits(std::vector<particle_sim>& particles);
    void CalculateRMS();
    int64 mybegin, myend;
    void SetMPI(int num_ranks_, int rank)
    {
        this->num_ranks_ = num_ranks_;
        this->rank = rank;
    };

    //MPI version
    //float CalculateRMSPerSlice();
    void CalculateSlicesRMS(std::vector<VALS>& vals);
    void ReadPoints(std::vector<particle_sim>& particles);
    double GetSigma() { return sigma; }
    double GetRMS() { return rms; }
    double GetMedia() { return media; }
    long GetEntries() { return npix; }

    float GetMin() { return datamin; }
    float GetMax() { return datamax; }

    float GetCrval(int i)
    {
        if (i < 3)
            return crval[i];
        else
            return -1;
    }
    float GetCpix(int i)
    {
        if (i < 3)
            return cpix[i];
        else
            return -1;
    }
    float GetCdelt(int i)
    {
        if (i < 3)
            return cdelt[i];
        else
            return -1;
    }

    int GetNaxes(int i);

    // std::vector<particle_sim> particles;

    std::string getSpecies() { return species; };
    std::string getTransition() { return transition; };
    std::string getSurvey() { return survey; };

    void setSpecies(std::string s) { species = s; };
    void setTransition(std::string s) { transition = s; };
    void setSurvey(std::string s) { survey = s; };

protected:
    std::string survey;
    std::string species;
    std::string transition;

    float datamin;
    float datamax;
    float epoch; // Part of FITS Header file
    int* dimensions; // [x,y,z]
    int naxis;
    int points;
    double crval[3];
    double cpix[3];
    double cdelt[3];
    double sigma = 0;
    double media = 0;
    double rms = 0;
    long npix = 0;
    long naxes[3];
    int nSlices;

private:
    int num_ranks_, rank;

    std::string filename;
    char title[80];
    char xStr[80];
    char yStr[80];
    char zStr[80];

    std::vector<float> m_Rms;

    //    char cunit3[1][200];

    void ReadHeader();
    void printerror(int status); // from fitsio distribution
    void ComputeTF(COLOUR& e, float I, int typeS, int k);
    void NormaliseIR(float& r, float& I);
    double initSlice;
};

#endif
