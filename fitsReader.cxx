#include "fitsReader.h"

#include <cmath>

#include "cxxsupport/share_utils.h"
#include "cxxsupport/string_utils.h"
#include <algorithm>
#include <cctype>
#include <iostream> // std::cout
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>

/*
 *
// The fits reading functions here are from fitsio examples in cookbook.c
*/

//----------------------------------------------------------------------------
fitsReader::fitsReader()
{
    this->filename[0] = '\0';
    this->xStr[0] = '\0';
    this->yStr[0] = '\0';
    this->zStr[0] = '\0';
    this->title[0] = '\0';
    this->is3D = true;

    for (int i = 0; i < 3; i++) {
        crval[i] = 0;
        cpix[i] = 0;
        cdelt[i] = 0;
        naxes[i] = 10;
    }

    m_type = 0;
    nSlices = 100;
}

//----------------------------------------------------------------------------
fitsReader::~fitsReader()
{
}

void fitsReader::SetFileName(std::string name)
{

    if (name.empty()) {
        printf("No Datafile\n");
        return;
    }

    filename = name;
}

void fitsReader::ComputeTF(COLOUR& e, float I, int typeF, int k)
{
    /*
 * Sort of TF currently
*/

    //float I=log(I2);
    float rm = rms_koef * rms; //4.5*m_Rms[k];
    float step = (datamax - rm) / 20;
    switch (typeF) {
    case 0: {
        if ((I > rm) && (I <= (5 * step + rm)))
            e.g = (I - rm) / (2 * step);

        if ((I > (5 * step + rm)) && (I <= (8 * step + rm))) {
            e.r = 1.0;
            e.g = (I - 5 * step - rm) / (3 * step);
        }

        if ((I > (8 * step + rm)) && (I <= (11 * step + rm))) {
            e.r = 1.0;
            e.b = (I - 8 * step - rm) / (3 * step);
        }
        if ((I > (11 * step + rm)) && (I <= (15 * step + rm))) {
            e.b = 1.0;
            e.r = 1.0 - (I - 11 * step - rm) / (4 * step);
            e.g = (I - 11 * step - rm) / (4 * step);
        }
        if (I > (15 * step + rm)) {
            e.r = 0.0;
            e.g = 1.0 - (I - 15 * step - rm) / (5 * step);
            e.b = 1.0;
        }
        break;
    }
    case 1: {
        float m_n = 1.0 / (datamax - 3 * rms);

        e.g = (I - 3 * rms) / (datamax - 3 * rms);
        e.b = (I - 3 * rms) / (datamax - 3 * rms);
        e.r = (I - 3 * rms) / (datamax - 3 * rms);

        break;
    }
    }
}
void fitsReader::NormaliseIR(float& r, float& I)
{
    float scale = (datamax);
    float Il = I / scale;
    // TODO:Below are current patches with magic numbers not to change shaders
    r = exp(-Il) * 22; //27 //usually 50, introduce minimum
    // I*=0.1;
}

//----------------------------------------------------------------------------
void fitsReader::ReadFits(std::vector<particle_sim>& particles)
{
    //Calculating contour value to further through away the points
    std::vector<VALS> vals;

    CalculateRMS();
    CalculateSlicesRMS(vals);

    int num2 = 0; //number of particles

    float point[3];
    float bmin[3] = { 0, 0, 0 };
    
    float norm = fmax(naxes[0], naxes[1]);
    printf("norm=%f\n", norm);

    norm /= 100; ///=fmax(norm,naxes[1])/100;

    //todo get max and min velocity in z
    float bmax[3] = { naxes[0] / norm, naxes[1] / norm, float(naxes[2] / norm) }; //naxes[2]/norm
    
    
    float delta[3] = { 0.16, 0.16, 0.16 }; //{(bmax[0]-bmin[0])/naxes[0],(bmax[1]-bmin[1])/naxes[1],(bmax[2]-bmin[2])/naxes[2]};
    bmin[2]=float(mybegin)*delta[2];
    
    printf("del=%f,%f,%f\n", delta[1], delta[0], delta[2]);
    float normilise = naxes[0] / naxes[2];
    int i, j, k;
    i = 0;
    j = 0;
    k = 0;

    //For every pixel
    //  std::vector<VALS>::iterator it = vals.begin() ;
    // bool stop=false;
    //  while(k<naxes[2]/100)
    for (std::vector<VALS>::iterator it = vals.begin(); it != vals.end(); ++it)

    {

        float I = it->val;

        COLOUR e(1, 0, 0);
        //if((I>4*rms))//&&(I<=(7*rms))) //check if still should skip
        {
            //   I=0.00001;

            float rr = fmax(delta[0], delta[1]) / 2.0;

            float32 r = rr * 100;
            //NormaliseIR(r,I); //process after normalisation

            int type_S = 0;
            bool active = true;
            float p[3];

            float approx = float(it->ii) / float(naxes[1] * naxes[0]);
            k = int(floor(approx));
            float rest = approx - k;

            if (rest > 0) {
                int ii2 = it->ii - naxes[1] * naxes[0] * k;
                approx = float(ii2) / float(naxes[0]);
                j = int(floor(approx));
                if (approx - float(j) > 0) {
                    i = ii2 - naxes[0] * j;

                } else
                    i = 0;
            } else {
                j = 0;
                i = 0;
            }

            p[0] = bmin[0] + delta[0] * i;
            p[1] = bmin[1] + delta[1] * j;
            p[2] = bmin[2] + delta[2] * k;

            float norm = bmax[2] ;//- 0;//bmin[2];
            float vl = p[2]/norm;//(p[2] - bmin[2]) / norm;
            int typeF = m_type;
            if (m_type == 2)
                typeF = 0;
            // ComputeTF(e,vl*(datamax-3*rms)+3*rms,typeF);
            //ComputeTF(e,I,typeF);

            if (m_type < 2) {
                ComputeTF(e, I, typeF, k);
            } else {
                ComputeTF(e, vl * (datamax - 3 * rms) + 3 * rms, typeF, k);
            }
            //std::cout<<"nums "<< naxes[0]<<" "<< naxes[1]<<" "<< naxes[2]<<" "<<std::endl;

            //TODO: here I do flip of data
            //it is a patch, ideally I should not do it but something is
            //wrong with reading

            float scale_opacity = 1.0; //naxes[2]/150.0;// norm was used insscale opacity for small and large datasets

            particles.push_back(particle_sim(e, p[0], p[1], p[2], r, I * scale_opacity, type_S, active));
            num2++;
            //  it++;
        }
    }

    particles.resize(num2);
}

void fitsReader::ReadHeader()
{

    fitsfile* fptr; /* pointer to the FITS file */

    int status, nkeys, keypos, hdutype, ii, jj;
    char card[FLEN_CARD]; /* standard string lengths defined in fitsioc.h */

    char crval1[80];
    char crval2[80];
    char crval3[80];
    char crpix1[80];
    char crpix2[80];
    char crpix3[80];
    char cdelt1[80];
    char cdelt2[80];
    char cdelt3[80];
    char naxis1[80];
    char naxis2[80];
    char naxis3[80];

    crval1[0] = '\0';
    crval2[0] = '\0';
    crval3[0] = '\0';
    crpix1[0] = '\0';
    crpix2[0] = '\0';
    crpix3[0] = '\0';
    cdelt1[0] = '\0';
    cdelt2[0] = '\0';
    cdelt3[0] = '\0';

    std::string val1, val2, val3, pix1, pix2, pix3, delt1, delt2, delt3, nax1, nax2, nax3;

    status = 0;

    char* fn = new char[filename.length() + 1];
    ;
    strcpy(fn, filename.c_str());

    if (fits_open_file(&fptr, fn, READONLY, &status))
        printerror(status);
    delete[] fn;

    /* attempt to move to next HDU, until we get an EOF error */
    for (ii = 1; !(fits_movabs_hdu(fptr, ii, &hdutype, &status)); ii++) {

        /* get no. of keywords */
        if (fits_get_hdrpos(fptr, &nkeys, &keypos, &status))
            printerror(status);

        for (jj = 1; jj <= nkeys; jj++) {

            if (fits_read_record(fptr, jj, card, &status))
                printerror(status);

            if (!strncmp(card, "CTYPE", 5)) {

                char* first = strchr(card, '\'');
                char* last = strrchr(card, '\'');

                *last = '\0';
                if (card[5] == '1')
                    strcpy(xStr, first + 1);
                if (card[5] == '2')
                    strcpy(yStr, first + 1);
                if (card[5] == '3')
                    strcpy(zStr, first + 1);
            }

            if (!strncmp(card, "OBJECT", 6)) {
                std::cerr << card << std::endl;
                char* first = strchr(card, '\'');
                char* last = strrchr(card, '\'');
                *last = '\0';
                strcpy(title, first + 1);
            }

            if (!strncmp(card, "CRVAL", 5)) {
                char* first = strchr(card, '=');
                char* last = strrchr(card, '=');
                *last = '\0';

                // char *last = strrchr(card, '/');
                //*last = '\0';

                if (card[5] == '1') {
                    strcpy(crval1, first + 1);
                    char* pch = strtok(crval1, " ,");
                    strcpy(crval1, pch);
                }

                if (card[5] == '2') {
                    strcpy(crval2, first + 1);
                    char* pch = strtok(crval2, " ,");
                    strcpy(crval2, pch);
                }

                if (card[5] == '3') {
                    strcpy(crval3, first + 1);
                    char* pch = strtok(crval3, " ,");
                    strcpy(crval3, pch);
                }
            }

            if (!strncmp(card, "CRPIX", 5)) {
                char* first = strchr(card, '=');
                char* last = strrchr(card, '=');
                *last = '\0';

                if (card[5] == '1') {
                    strcpy(crpix1, first + 1);

                    char* pch = strtok(crpix1, " ,");
                    strcpy(crpix1, pch);
                }

                if (card[5] == '2') {
                    strcpy(crpix2, first + 1);

                    char* pch = strtok(crpix2, " ,");
                    strcpy(crpix2, pch);
                }
                if (card[5] == '3') {
                    strcpy(crpix3, first + 1);

                    char* pch = strtok(crpix3, " ,");
                    strcpy(crpix3, pch);
                }
            }

            if (!strncmp(card, "CDELT", 5)) {
                char* first = strchr(card, '=');
                char* last = strrchr(card, '=');
                *last = '\0';

                if (card[5] == '1') {
                    strcpy(cdelt1, first + 1);
                    char* pch = strtok(cdelt1, " ,");
                    strcpy(cdelt1, pch);
                }

                if (card[5] == '2') {
                    strcpy(cdelt2, first + 1);
                    char* pch = strtok(cdelt2, " ,");
                    strcpy(cdelt2, pch);
                }

                if (card[5] == '3') {
                    strcpy(cdelt3, first + 1);
                    char* pch = strtok(cdelt3, " ,");
                    strcpy(cdelt3, pch);
                }
            }
        }
    }

    val1 = crval1;
    val2 = crval2;
    val3 = crval3;
    pix1 = crpix1;
    pix2 = crpix2;
    pix3 = crpix3;
    delt1 = cdelt1;
    delt2 = cdelt2;
    delt3 = cdelt3;

    crval[0] = ::atof(val1.c_str());
    crval[1] = ::atof(val2.c_str());
    crval[2] = ::atof(val3.c_str());
    cpix[0] = ::atof(pix1.c_str());
    cpix[1] = ::atof(pix2.c_str());
    cpix[2] = ::atof(pix3.c_str());
    cdelt[0] = ::atof(delt1.c_str());
    cdelt[1] = ::atof(delt1.c_str());
    cdelt[2] = ::atof(delt1.c_str());

    initSlice = crval[2] - (cdelt[2] * (cpix[2] - 1));

    std::cout << crval[0] << " " << crval[1] << " " << crval[2] << " " << cpix[0] << " " << cpix[1] << " " << cpix[2] << " " << cdelt[0] << " " << cdelt[1] << " " << cdelt[2] << std::endl;
}

void fitsReader::printerror(int status)
{

    if (status) {
        fits_report_error(stderr, status); /* print error report */
        exit(status); /* terminate the program, returning error status */
    }
    return;
}

// TODO: first we read file to calculate RMS
void fitsReader::CalculateRMS()
{

    ReadHeader();

    fitsfile* fptr;
    int status = 0, nfound = 0, anynull = 0;
    long fpixel, nbuffer, npixels, ii, n = 0;
    double meansquare = 0;
    const int buffsize = naxes[0]; //later reduce to lower if needed by bigger files

    float nullval, buffer[buffsize];
    char* fn = new char[filename.length() + 1];
    strcpy(fn, filename.c_str());

    if (fits_open_file(&fptr, fn, READONLY, &status))
        printerror(status);

    delete[] fn;

    if (fits_read_keys_lng(fptr, "NAXIS", 1, 4, naxes, &nfound, &status))
        printerror(status);
    if (naxes[2]<=1)
        naxes[2]=naxes[3];
    minB[0]=0;
    minB[1]=0;
    minB[2]=0;
    maxB[0]=0.16*naxes[0]+1.0;
    maxB[1]=0.16*naxes[1]+1.0;
    maxB[2]=0.16*naxes[2]+1.0;
    
    //mpiMgr.calcShare (0, naxes[2], mybegin, myend);
    //void calcShare (int64 glo, int64 ghi, int64 &lo, int64 &hi) const
    //{
    calcShareGeneral(0, naxes[2], num_ranks_, rank, mybegin, myend);
    std::cout << rank << " reading " << mybegin << ", " << myend << " of total " << naxes[2] << std::endl;
    //int npart = nSlices * naxes[0] * naxes[1];
    //points.resize(npart);

    nSlices = myend - mybegin; //naxes[2]; //set currently number of slices

    //int sliceNum=5; //equal to number of processors
    //int sliceNum2=5;
    // int numSl=100;
    // std::vector<float> rms;

    //int sliceN=1;

    //npixels  = naxes[0] * naxes[1]*100;// * naxes[2]/sliceNum; //pixels per clice

    n = 0;

    fpixel = 1; //+naxes[0] * naxes[1];//+npixels/sliceNum*sliceN; //start from slice
    nullval = 0;
    datamin = 1.0E30;
    datamax = -1.0E30;

    int bad = 0;

    //For every pixel

    int num2 = 0;

    m_Rms.reserve(nSlices); // rms per slice
    //For every pixel
    npixels = naxes[0] * naxes[1]; // * naxes[2]/sliceNum; //pixels per clice
    fpixel = naxes[0] * naxes[1] * mybegin + 1; //start from the element

    for (int k = 0; k < nSlices; k++) {

        npixels = naxes[0] * naxes[1]; // * naxes[2]/sliceNum; //pixels per clice
        int numGoodPix = 0;
        m_Rms[k] = 0;
        while (npixels > 0) {

            nbuffer = npixels;
            if (npixels > buffsize)
                nbuffer = buffsize;

            if (fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                    buffer, &anynull, &status))
                printerror(status);

            for (ii = 0; ii < nbuffer; ii++) {

                if (!std::isnan(buffer[ii]) && (buffer[ii] >= 0.0f)) {

                    /* if ( buffer[ii] < datamin )
                    datamin = buffer[ii];
                if ( buffer[ii] > datamax   )
                    datamax = buffer[ii];*/

                    meansquare += buffer[ii] * buffer[ii];
                    //vals.push_back(VALS(num2,buffer[ii]));
                    n++;

                    m_Rms[k] += buffer[ii] * buffer[ii];
                    numGoodPix++;
                }

                num2++;
            }
            npixels -= nbuffer;
            fpixel += nbuffer;
        }
        m_Rms[k] /= numGoodPix;
        m_Rms[k] = sqrt(m_Rms[k]);
    }

    printf("pixesl=%d\n", n);
    double means = meansquare / n;
    rms = sqrt(means);

    if (fits_close_file(fptr, &status))
        printerror(status);

    return;
}

// TODO: first we read file to calculate RMS

// TODO: first we read file to calculate RMS
void fitsReader::CalculateSlicesRMS(std::vector<VALS>& vals)
{

    ReadHeader();

    fitsfile* fptr;
    int status = 0, nfound = 0, anynull = 0;
    long fpixel, nbuffer, npixels, ii, n = 0;
    double meansquare = 0;
    const int buffsize = naxes[0]; //later reduce to lower if needed by bigger files

    float nullval, buffer[buffsize];
    char* fn = new char[filename.length() + 1];
    strcpy(fn, filename.c_str());

    if (fits_open_file(&fptr, fn, READONLY, &status))
        printerror(status);

    delete[] fn;

    if (fits_read_keys_lng(fptr, "NAXIS", 1, 3, naxes, &nfound, &status))
        printerror(status);

    n = 0;

    fpixel = naxes[0] * naxes[1] * mybegin + 1; //+naxes[0] * naxes[1];//+npixels/sliceNum*sliceN; //start from slice
    nullval = 0;
    datamin = 1.0E30;
    datamax = -1.0E30;

    //For every pixel

    int num2 = 0;

    //For every pixel
    npixels = naxes[0] * naxes[1]; // * naxes[2]/sliceNum; //pixels per clice

    for (int k = 0; k < nSlices; k++) {

        npixels = naxes[0] * naxes[1]; // * naxes[2]/sliceNum; //pixels per clice
        //printf("rms=%d, %lf\n",k,m_Rms[k]);

        while (npixels > 0) {

            nbuffer = npixels;
            if (npixels > buffsize)
                nbuffer = buffsize;

            if (fits_read_img(fptr, TFLOAT, fpixel, nbuffer, &nullval,
                    buffer, &anynull, &status))
                printerror(status);

            for (ii = 0; ii < nbuffer; ii++) {

                if (!std::isnan(buffer[ii]) && (buffer[ii] >= 0.0f)) {

                    /* if ( buffer[ii] < datamin )
                    datamin = buffer[ii];
                if ( buffer[ii] > datamax   )
                    datamax = buffer[ii];

                meansquare+=buffer[ii]*buffer[ii];*/
                    if (buffer[ii] > rms_koef * m_Rms[k]) {
                        vals.push_back(VALS(num2, buffer[ii]));
                        if (buffer[ii] < datamin)
                            datamin = buffer[ii];
                        if (buffer[ii] > datamax)
                            datamax = buffer[ii];
                    }
                }

                num2++;
            }
            npixels -= nbuffer;
            fpixel += nbuffer;
        }
    }

    if (fits_close_file(fptr, &status))
        printerror(status);

    return;
}
int fitsReader::GetNaxes(int i)
{

    return naxes[i];
}
