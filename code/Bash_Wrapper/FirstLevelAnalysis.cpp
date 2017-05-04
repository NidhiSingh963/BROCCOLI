/*
 * BROCCOLI: Software for Fast fMRI Analysis on Many-Core CPUs and GPUs
 * Copyright (C) <2013>  Anders Eklund, andek034@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "broccoli_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include "nifti1_io.h"
#include <iostream>
#include <fstream>
#include <iomanip>

#include "HelpFunctions.cpp"

#define ADD_FILENAME true
#define DONT_ADD_FILENAME true

#define CHECK_EXISTING_FILE true
#define DONT_CHECK_EXISTING_FILE false


int main(int argc, char **argv)
{
    //-----------------------
    // Input pointers
    
    float           *h_fMRI_Volumes, *h_fMRI_Volumes_MNI, *h_T1_Volume, *h_Interpolated_T1_Volume, *h_Aligned_T1_Volume_Linear, *h_Aligned_T1_Volume_NonLinear, *h_MNI_Volume, *h_MNI_Brain_Volume, *h_MNI_Brain_Mask, *h_Aligned_EPI_Volume_T1, *h_Aligned_EPI_Volume_MNI_Linear, *h_Aligned_EPI_Volume_MNI_Nonlinear; 
  
    float           *h_Quadrature_Filter_1_Linear_Registration_Real, *h_Quadrature_Filter_2_Linear_Registration_Real, *h_Quadrature_Filter_3_Linear_Registration_Real, *h_Quadrature_Filter_1_Linear_Registration_Imag, *h_Quadrature_Filter_2_Linear_Registration_Imag, *h_Quadrature_Filter_3_Linear_Registration_Imag;
    float           *h_Quadrature_Filter_1_NonLinear_Registration_Real, *h_Quadrature_Filter_2_NonLinear_Registration_Real, *h_Quadrature_Filter_3_NonLinear_Registration_Real, *h_Quadrature_Filter_1_NonLinear_Registration_Imag, *h_Quadrature_Filter_2_NonLinear_Registration_Imag, *h_Quadrature_Filter_3_NonLinear_Registration_Imag;
    float           *h_Quadrature_Filter_4_NonLinear_Registration_Real, *h_Quadrature_Filter_5_NonLinear_Registration_Real, *h_Quadrature_Filter_6_NonLinear_Registration_Real, *h_Quadrature_Filter_4_NonLinear_Registration_Imag, *h_Quadrature_Filter_5_NonLinear_Registration_Imag, *h_Quadrature_Filter_6_NonLinear_Registration_Imag;
  
    int             IMAGE_REGISTRATION_FILTER_SIZE = 7;
    int             NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_RIGID = 6;
    int             NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_AFFINE = 12;
    
    float           h_T1_MNI_Registration_Parameters[NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_AFFINE];
    float           h_EPI_T1_Registration_Parameters[NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_RIGID];
    float           h_EPI_MNI_Registration_Parameters[NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_AFFINE];
    float           *h_Motion_Parameters;
    
    float           *h_Projection_Tensor_1, *h_Projection_Tensor_2, *h_Projection_Tensor_3, *h_Projection_Tensor_4, *h_Projection_Tensor_5, *h_Projection_Tensor_6;    
    
    float           *h_Filter_Directions_X, *h_Filter_Directions_Y, *h_Filter_Directions_Z;
    
	float			*h_EPI_Mask, *h_MNI_Mask;
    float           *h_Slice_Timing_Corrected_fMRI_Volumes;
    float           *h_Motion_Corrected_fMRI_Volumes;
    float           *h_Smoothed_fMRI_Volumes;    
	float			h_Custom_Slice_Times[1000];
    
    float           *h_X_GLM, *h_xtxxt_GLM, *h_X_GLM_Confounds, *h_Contrasts, *h_ctxtxc_GLM, *h_Highres_Regressors, *h_LowpassFiltered_Regressors;
    unsigned short int        *h_Permutation_Matrix;
    float           *h_Permutation_Distribution;

	float			*h_Beta_Volumes_EPI, *h_Contrast_Volumes_EPI, *h_Statistical_Maps_EPI, *h_P_Values_EPI;
	float			*h_Beta_Volumes_T1, *h_Contrast_Volumes_T1, *h_Statistical_Maps_T1, *h_P_Values_T1;        
	float			*h_Beta_Volumes_MNI, *h_Contrast_Volumes_MNI, *h_Statistical_Maps_MNI, *h_P_Values_MNI;

	float			*h_Beta_Volumes_No_Whitening_EPI, *h_Contrast_Volumes_No_Whitening_EPI, *h_Statistical_Maps_No_Whitening_EPI;    
	float			*h_Beta_Volumes_No_Whitening_T1, *h_Contrast_Volumes_No_Whitening_T1, *h_Statistical_Maps_No_Whitening_T1;    
	float			*h_Beta_Volumes_No_Whitening_MNI, *h_Contrast_Volumes_No_Whitening_MNI, *h_Statistical_Maps_No_Whitening_MNI;

    float           *h_AR1_Estimates_EPI, *h_AR2_Estimates_EPI, *h_AR3_Estimates_EPI, *h_AR4_Estimates_EPI;
    float           *h_AR1_Estimates_T1, *h_AR2_Estimates_T1, *h_AR3_Estimates_T1, *h_AR4_Estimates_T1;
    float           *h_AR1_Estimates_MNI, *h_AR2_Estimates_MNI, *h_AR3_Estimates_MNI, *h_AR4_Estimates_MNI;
        
	float			*h_Residuals_EPI;
	float			*h_Residuals_MNI;

    size_t          EPI_DATA_W, EPI_DATA_H, EPI_DATA_D, EPI_DATA_T, NUMBER_OF_RUNS;
	size_t*			EPI_DATA_T_PER_RUN;
    size_t          T1_DATA_H, T1_DATA_W, T1_DATA_D;
    size_t          MNI_DATA_W, MNI_DATA_H, MNI_DATA_D;
                
    float           EPI_VOXEL_SIZE_X, EPI_VOXEL_SIZE_Y, EPI_VOXEL_SIZE_Z, TR;
    float           T1_VOXEL_SIZE_X, T1_VOXEL_SIZE_Y, T1_VOXEL_SIZE_Z;
    float           MNI_VOXEL_SIZE_X, MNI_VOXEL_SIZE_Y, MNI_VOXEL_SIZE_Z;
    
    size_t          NUMBER_OF_GLM_REGRESSORS, NUMBER_OF_TOTAL_GLM_REGRESSORS, NUMBER_OF_CONFOUND_REGRESSORS, NUMBER_OF_CONTRASTS;
    
    size_t          NUMBER_OF_DETRENDING_REGRESSORS = 4;
    size_t          NUMBER_OF_MOTION_REGRESSORS = 6;	

	int				NUMBER_OF_EVENTS;
	size_t			HIGHRES_FACTOR = 100;
    
    //-----------------------
    // Output pointers
    
    float           *h_Design_Matrix, *h_Design_Matrix2;
    float           *h_Whitened_Models;
    
    //----------
    
    void*           allMemoryPointers[500];
	for (int i = 0; i < 500; i++)
	{
		allMemoryPointers[i] = NULL;
	}
    
	nifti_image*	allNiftiImages[500];
	for (int i = 0; i < 500; i++)
	{
		allNiftiImages[i] = NULL;
	}

    int             numberOfMemoryPointers = 0;
	int				numberOfNiftiImages = 0;

	size_t			allocatedHostMemory = 0;
    
    //---------------------
    // Settings
    
    int             OPENCL_PLATFORM = 0;
    int             OPENCL_DEVICE = 0;
    
    int             NUMBER_OF_ITERATIONS_FOR_LINEAR_IMAGE_REGISTRATION = 10;
    int             NUMBER_OF_ITERATIONS_FOR_NONLINEAR_IMAGE_REGISTRATION = 10;
    int 			COARSEST_SCALE_T1_MNI = 4;
	int				COARSEST_SCALE_EPI_T1 = 4;
	int				MM_T1_Z_CUT = 0;
	int				MM_EPI_Z_CUT = 0;
    float           SIGMA = 5.0f;
    
	bool			APPLY_SLICE_TIMING_CORRECTION = true;
	bool			APPLY_MOTION_CORRECTION = true;
	bool			APPLY_SMOOTHING = true;

	int				SLICE_ORDER = UNDEFINED;
	bool			DEFINED_SLICE_PATTERN = false;
	bool			DEFINED_SLICE_CUSTOM_REF = false;
	int				SLICE_CUSTOM_REF = 0;
    int             NUMBER_OF_ITERATIONS_FOR_MOTION_CORRECTION = 5;

	bool			FOUND_REGRESSORS = false;

	bool			RAW_REGRESSORS = false;
	bool			RAW_DESIGNMATRIX = false;
    size_t          REGRESS_MOTION = 0;
    size_t          REGRESS_GLOBALMEAN = 0;
	size_t			REGRESS_CONFOUNDS = 0;
    float           EPI_SMOOTHING_AMOUNT = 6.0f;
    float           AR_SMOOTHING_AMOUNT = 6.0f;
	bool			BETAS_ONLY = false;
	bool			REGRESS_ONLY = false;
	bool			PREPROCESSING_ONLY = false;
	bool			MULTIPLE_RUNS = false;    
					NUMBER_OF_RUNS = 1;

	bool			CHANGE_OUTPUT_FILENAME = false;
	const char      *outputFilename;

    size_t          USE_TEMPORAL_DERIVATIVES = 0;
    bool            PERMUTE = false;
    size_t			NUMBER_OF_PERMUTATIONS = 1000;

    int				INFERENCE_MODE = 1;
    float           CLUSTER_DEFINING_THRESHOLD = 2.5f;
    bool            BAYESIAN = false;
    int             NUMBER_OF_MCMC_ITERATIONS = 1000;
	bool			MASK = false;
	const char*		MASK_NAME;
	const char*		SLICE_TIMINGS_FILE;
    float			SIGNIFICANCE_LEVEL = 0.05f;
	int				TEST_STATISTICS = 0;
    
	bool			WRITE_TRANSFORMATION_MATRICES = false;
    bool            WRITE_INTERPOLATED_T1 = false;
    bool            WRITE_ALIGNED_T1_MNI_LINEAR = false;
    bool            WRITE_ALIGNED_T1_MNI_NONLINEAR = false;
    bool            WRITE_ALIGNED_EPI_T1 = false;
    bool            WRITE_ALIGNED_EPI_MNI = false;
	bool			WRITE_EPI_MASK = false;
	bool			WRITE_MNI_MASK = false;
    bool            WRITE_SLICETIMING_CORRECTED = false;
    bool            WRITE_MOTION_CORRECTED = false;
	bool			WRITE_MOTION_PARAMETERS = false;
    bool            WRITE_SMOOTHED = false;
    bool            WRITE_ACTIVITY_EPI = false;
    bool            WRITE_ACTIVITY_T1 = false;
    bool            WRITE_RESIDUALS_EPI = false;
    bool            WRITE_RESIDUALS_MNI = false;
    bool            WRITE_DESIGNMATRIX = false;
	bool			WRITE_ORIGINAL_DESIGNMATRIX = false;
    bool            WRITE_AR_ESTIMATES_EPI = false;
    bool            WRITE_AR_ESTIMATES_T1 = false;
    bool            WRITE_AR_ESTIMATES_MNI = false;
	bool			WRITE_UNWHITENED_RESULTS = false;
	bool			WRITE_COMPACT = false;    

    bool            PRINT = true;
    bool            VERBOS = false;
    bool            DEBUG = false;
    
    //---------------------    
   
    /* Input arguments */
    FILE *fp = NULL;
    
    // No inputs, so print help text
    if (argc == 1)
    {   
		printf("\nThe function performs first level analysis of one fMRI dataset. The processing includes registration between T1 and MNI, registration between fMRI and T1, slice timing correction, motion correction, smoothing and statistical analysis. \n\n");     
        printf("Usage, preprocessing + GLM, single run:\n\n");
        printf("FirstLevelAnalysis fMRI_data.nii T1_volume.nii MNI_volume.nii regressors.txt contrasts.txt [options]\n\n");

        printf("Usage, preprocessing + GLM, three runs:\n\n");
        printf("FirstLevelAnalysis -runs 3 run1.nii run2.nii run3.nii T1_volume.nii MNI_volume.nii regressors_run1.txt regressors_run2.txt regressors_run3.txt contrasts.txt [options]\n\n");

        printf("Usage, preprocessing + regress nuisance variables (mean, trends, (motion), (global mean)), single run:\n\n");
        printf("FirstLevelAnalysis fMRI_data.nii T1_volume.nii MNI_volume.nii -regressonly [options]\n\n");

        printf("Usage, preprocessing + regress nuisance variables (mean, trends, (motion), (global mean)), three runs:\n\n");
        printf("FirstLevelAnalysis -runs 3 run1.nii run2.nii run3.nii T1_volume.nii MNI_volume.nii -regressonly [options]\n\n");

        printf("Usage, preprocessing only (no GLM):\n\n");
        printf("FirstLevelAnalysis fMRI_data.nii T1_volume.nii MNI_volume.nii -preprocessingonly [options]\n\n");
        
        printf("OpenCL options:\n\n");
        printf(" -platform                  The OpenCL platform to use (default 0) \n");
        printf(" -device                    The OpenCL device to use for the specificed platform (default 0) \n\n");
        
        printf("Registration options:\n\n");
        printf(" -iterationslinear          Number of iterations for the linear registration (default 10) \n");        
        printf(" -iterationsnonlinear       Number of iterations for the non-linear registration (default 10), 0 means that no non-linear registration is performed \n");        
        //printf(" -lowestscalet1             The lowest scale for the linear and non-linear registration of the T1 volume to MNI, should be 1, 2, 4 or 8 (default 4), x means downsampling a factor x in each dimension  \n");        
        //printf(" -lowestscaleepi            The lowest scale for the linear registration of the fMRI volume to the T1 volume, should be 1, 2, 4 or 8 (default 4), x means downsampling a factor x in each dimension  \n");        
        printf(" -zcutt1                    Number of mm to cut from the bottom of the T1 volume, can be negative, useful if the head in the volume is placed very high or low (default 0) \n\n");
        printf(" -zcutepi                   Number of mm to cut from the bottom of the fMRI volume, can be negative, useful if the head in the volume is placed very high or low (default 0) \n");
        printf(" -sigma                     Amount of Gaussian smoothing applied for regularization of the displacement field, defined as sigma of the Gaussian kernel (default 5.0)  \n\n\n\n");        
        
        printf("Preprocessing options:\n\n");
        printf(" -noslicetimingcorrection   Do not apply slice timing correction\n");
        printf(" -nomotioncorrection        Do not apply motion correction\n");
        printf(" -nosmoothing               Do not apply any smoothing\n\n");

        printf(" -slicepattern              The sampling pattern used during scanning (overrides pattern provided in NIFTI file)\n");
		printf("                            0 = sequential 1-N (bottom-up), 1 = sequential N-1 (top-down), 2 = interleaved 1-N, 3 = interleaved N-1 \n");
		printf("                            (no slice timing correction is performed if pattern in NIFTI file is unknown and no pattern is provided) \n");        
        printf(" -slicecustom               Provide a text file with the slice times, one value per slice, in milli seconds (0 - TR) (overrides pattern provided in NIFTI file)\n");
		printf(" -slicecustomref            Reference slice for the custom slice times (0 - (#slices-1)) (default #slices/2)\n");
        printf(" -iterationsmc              Number of iterations for motion correction (default 5) \n");
        printf(" -smoothing                 Amount of smoothing to apply to the fMRI data (default 6.0 mm) \n\n");
        
        printf("Statistical options:\n\n");
        printf(" -runs                      Analyze more than one run, provide number of runs (default false). \n");
        printf(" -preprocessingonly         Only perform preprocessing, no GLM or regression is performed (default no). \n");
		printf(" -detrendingregressors      Set the number of detrending regressors, 1-4 (default 4) \n");
        printf(" -betasonly                 Only perform preprocessing and calculate beta values and contrasts, no t- or F-scores are calculated (default no). \n");
        printf(" -regressonly               Only perform preprocessing and regress nuisance variables, no beta values or contrasts are calculated (default no). \n");
		printf("                            Regressor and contrast file not needed. \n");
        printf(" -rawregressors             Use raw regressors (FSL format, one row per TR) (default no) \n");
        printf(" -rawdesignmatrix           Provide the design matrix in a single text file (FSL format, one regressor per column, one value per TR) (default no) \n");
        printf(" -regressmotion             Include motion parameters in design matrix (default no) \n");
        printf(" -regressglobalmean         Include global mean in design matrix (default no) \n");
        printf(" -temporalderivatives       Use temporal derivatives for the activity regressors (default no) \n");
        printf(" -permute                   Apply a permutation test to get p-values (default no) \n");
        printf(" -permutations              Number of permutations to use for permutation test (default 1,000) \n");
        printf(" -inferencemode             Inference mode to use for permutation test, 0 = voxel, 1 = cluster extent, 2 = cluster mass, 3 = TFCE (default 1) \n");
        printf(" -cdt                       Cluster defining threshold for cluster inference (default 2.5) \n");
        printf(" -bayesian                  Do Bayesian analysis using MCMC, currently only supports 2 regressors (default no) \n");
        printf(" -iterationsmcmc            Number of iterations for MCMC chains (default 1,000) \n");
        printf(" -mask                      Apply a mask to the statistical maps after the statistical analysis, in MNI space (default none) \n\n");

        printf("Misc options:\n\n");
        //printf(" -savecompact               Save beta values, contrast results and t-/F-scores as single files, instead of one per regressor/contrast (default no)\n");
        printf(" -savetransformations       Save all affine transformation matrices (T1-MNI,EPI-T1,EPI-MNI) (default no) \n");
        printf(" -savet1interpolated        Save T1 volume after resampling to MNI voxel size and resizing to MNI size (default no) \n");
        printf(" -savet1alignedlinear       Save T1 volume linearly aligned to the MNI volume (default no) \n");
        printf(" -savet1alignednonlinear    Save T1 volume non-linearly aligned to the MNI volume (default no) \n");
        printf(" -saveepialignedt1          Save EPI volume aligned to the T1 volume (default no) \n");
        printf(" -saveepialignedmni         Save EPI volume aligned to the MNI volume (default no) \n");
        printf(" -saveepimask               Save EPI mask for fMRI data  (default no) \n");
        printf(" -savemnimask               Save MNI mask for fMRI data  (default no) \n");
        printf(" -saveslicetimingcorrected  Save slice timing corrected fMRI volumes  (default no) \n");
        printf(" -savemotioncorrected       Save motion corrected fMRI volumes (default no) \n");
        printf(" -savemotionparameters      Save motion parameters as a text file (default no) \n");
        printf(" -savesmoothed              Save smoothed fMRI volumes (default no) \n");
        printf(" -saveactivityepi           Save activity maps in EPI space (in addition to MNI space, default no) \n");
        printf(" -saveactivityt1            Save activity maps in T1 space (in addition to MNI space, default no) \n");
        printf(" -saveresiduals             Save residuals after GLM analysis (default no) \n");
        printf(" -saveresidualsmni          Save residuals after GLM analysis, in MNI space (default no) \n");
        printf(" -saveoriginaldesignmatrix  Save the original design matrix used (default no) \n");
        printf(" -savedesignmatrix          Save the total design matrix used (default no) \n");
        printf(" -savearparameters          Save the estimated AR coefficients (default no) \n");
        printf(" -savearparameterst1        Save the estimated AR coefficients, in T1 space (default no) \n");
        printf(" -savearparametersmni       Save the estimated AR coefficients, in MNI space (default no) \n");
        printf(" -saveallaligned            Save all aligned volumes (T1 interpolated, T1-MNI linear, T1-MNI non-linear, EPI-T1, EPI-MNI) (default no) \n");
        printf(" -saveallpreprocessed       Save all preprocessed fMRI data (slice timing corrected, motion corrected, smoothed) (default no) \n");
        printf(" -saveunwhitenedresults     Save all statistical results without voxel-wise whitening (default no) \n");
        printf(" -saveall                   Save everything (default no) \n");
        printf(" -output                    Set output filename (default fMRI*.nii) \n");
        printf(" -quiet                     Don't print anything to the terminal (default false) \n");
        printf(" -verbose                   Print extra stuff (default false) \n");
        printf(" -debug                     Get additional debug information saved as nifti files (default no). Warning: This will use a lot of extra memory! \n");
        printf("\n\n");
        
        return EXIT_SUCCESS;
    }

	// Check if first argument is -runs
	char *temp = argv[1];
    if (strcmp(temp,"-runs") == 0)
    {		
        MULTIPLE_RUNS = true;

		char *p;
		NUMBER_OF_RUNS = (int)strtol(argv[2], &p, 10);
			
		if (!isspace(*p) && *p != 0)
	    {
	        printf("Number of runs must be an integer! You provided %s \n",argv[2]);
			return EXIT_FAILURE;
	    }
        else if (NUMBER_OF_RUNS < 1)
        {
			printf("Number of runs must be > 1!\n");
            return EXIT_FAILURE;
        }
    }

	EPI_DATA_T_PER_RUN = (size_t*)malloc(NUMBER_OF_RUNS*sizeof(size_t));

	// Check if 4'th argument is -regressonly or -preprocessingonly
	if (!MULTIPLE_RUNS)
	{
		char *temp = argv[4];
    	if (strcmp(temp,"-regressonly") == 0)
    	{
    	    REGRESS_ONLY = true;
    	}
		else if (strcmp(temp,"-preprocessingonly") == 0)
		{
    	    PREPROCESSING_ONLY = true;
		}
	}
	else
	{
		char *temp = argv[5 + NUMBER_OF_RUNS];
    	if (strcmp(temp,"-regressonly") == 0)
    	{
    	    REGRESS_ONLY = true;
    	}
		else if (strcmp(temp,"-preprocessingonly") == 0)
		{
    	    PREPROCESSING_ONLY = true;
		}
	}

	int i;
	// Try to open all files
	if (!MULTIPLE_RUNS)
	{
		if (REGRESS_ONLY || PREPROCESSING_ONLY)
		{
    	    for (int j = 1; j <= 3; j++)
    	    {
    	        fp = fopen(argv[j],"r");
    	        if (fp == NULL)
    	        {
    	            printf("Could not open file %s !\n",argv[j]);
    	            return EXIT_FAILURE;
    	        }
    	        fclose(fp);
    	    }
			i = 5;
		}
    	else
    	{
    	    for (int j = 1; j <= 5; j++)
    	    {
    	        fp = fopen(argv[j],"r");
    	        if (fp == NULL)
    	        {
    	            printf("Could not open file %s !\n",argv[j]);
    	            return EXIT_FAILURE;
    	        }
    	        fclose(fp);
    	    }
			i = 6;
    	}
	}
	else
	{
		if (REGRESS_ONLY || PREPROCESSING_ONLY)
		{
    	    for (int j = 3; j <= (NUMBER_OF_RUNS + 4); j++)
    	    {
    	        fp = fopen(argv[j],"r");
    	        if (fp == NULL)
    	        {
    	            printf("Could not open file %s !\n",argv[j]);
    	            return EXIT_FAILURE;
    	        }
    	        fclose(fp);
    	    }
			i = 6 + NUMBER_OF_RUNS;
		}
    	else
    	{
    	    for (int j = 3; j <= (5 + NUMBER_OF_RUNS*2); j++)
    	    {
    	        fp = fopen(argv[j],"r");
    	        if (fp == NULL)
    	        {
    	            printf("Could not open file %s !\n",argv[j]);
    	            return EXIT_FAILURE;
    	        }
    	        fclose(fp);
    	    }
			i = 6 + NUMBER_OF_RUNS*2;
    	}

	}
    
    // Loop over additional inputs
    
    while (i < argc)
    {
        char *input = argv[i];
        char *p;
        
        // OpenCL options
        if (strcmp(input,"-platform") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -platform !\n");
                return EXIT_FAILURE;
			}

            OPENCL_PLATFORM = (int)strtol(argv[i+1], &p, 10);
			
			if (!isspace(*p) && *p != 0)
		    {
		        printf("OpenCL platform must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (OPENCL_PLATFORM < 0)
            {
                printf("OpenCL platform must be >= 0!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-device") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -device !\n");
                return EXIT_FAILURE;
			}

            OPENCL_DEVICE = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("OpenCL device must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (OPENCL_DEVICE < 0)
            {
                printf("OpenCL device must be >= 0!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        
        // Registration options
        else if (strcmp(input,"-iterationslinear") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -iterationslinear !\n");
                return EXIT_FAILURE;
			}

            NUMBER_OF_ITERATIONS_FOR_LINEAR_IMAGE_REGISTRATION = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of linear iterations must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (NUMBER_OF_ITERATIONS_FOR_LINEAR_IMAGE_REGISTRATION <= 0)
            {
                printf("Number of linear iterations must be a positive number!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-iterationsnonlinear") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -iterationsnonlinear !\n");
                return EXIT_FAILURE;
			}

            NUMBER_OF_ITERATIONS_FOR_NONLINEAR_IMAGE_REGISTRATION = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of non-linear iterations must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (NUMBER_OF_ITERATIONS_FOR_NONLINEAR_IMAGE_REGISTRATION < 0)
            {
                printf("Number of non-linear iterations must be >= 0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
		/*
        else if (strcmp(input,"-lowestscalet1") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -lowestscalet1 !\n");
                return EXIT_FAILURE;
			}

            COARSEST_SCALE_T1_MNI = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Lowest scale must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
  			else if ( (COARSEST_SCALE_T1_MNI != 1) && (COARSEST_SCALE_T1_MNI != 2) && (COARSEST_SCALE_T1_MNI != 4) && (COARSEST_SCALE_T1_MNI != 8) )
            {
                printf("Lowest scale must be 1, 2, 4 or 8!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-lowestscaleepi") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -lowestscaleepi !\n");
                return EXIT_FAILURE;
			}

            COARSEST_SCALE_EPI_T1 = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Lowest scale must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
  			else if ( (COARSEST_SCALE_EPI_T1 != 1) && (COARSEST_SCALE_EPI_T1 != 2) && (COARSEST_SCALE_EPI_T1 != 4) && (COARSEST_SCALE_EPI_T1 != 8) )
            {
                printf("Lowest scale must be 1, 2, 4 or 8!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
		*/
 		else if (strcmp(input,"-zcutt1") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -zcutt1 !\n");
                return EXIT_FAILURE;
			}

            MM_T1_Z_CUT = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("zcutt1 must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }

            i += 2;
        }
        else if (strcmp(input,"-zcutepi") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -zcutepi !\n");
                return EXIT_FAILURE;
			}

            MM_EPI_Z_CUT = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("zcutepi must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }

            i += 2;
        }
        else if (strcmp(input,"-sigma") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -sigma !\n");
                return EXIT_FAILURE;
			}

            SIGMA = (float)strtod(argv[i+1], &p);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("sigma must be a float! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
  			else if ( SIGMA < 0.0f )
            {
                printf("sigma must be >= 0.0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }      
        
        // Preprocessing options
        else if (strcmp(input,"-noslicetimingcorrection") == 0)
        {
			APPLY_SLICE_TIMING_CORRECTION = false;
			i += 1;
		}
        else if (strcmp(input,"-nomotioncorrection") == 0)
        {
			APPLY_MOTION_CORRECTION = false;
			i += 1;
		}
        else if (strcmp(input,"-nosmoothing") == 0)
        {
			APPLY_SMOOTHING = false;
			i += 1;
		}
        else if (strcmp(input,"-slicepattern") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicepattern !\n");
                return EXIT_FAILURE;
			}

            SLICE_ORDER = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Slice pattern must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (SLICE_ORDER < 0)
            {
                printf("Slice pattern must be a positive number!\n");
                return EXIT_FAILURE;
            }
            else if ( (SLICE_ORDER != 0) && (SLICE_ORDER != 1) && (SLICE_ORDER != 2) && (SLICE_ORDER != 3) )
            {
                printf("Slice pattern must be 0, 1, 2 or 3!\n");
                return EXIT_FAILURE;
            }
            i += 2;
			DEFINED_SLICE_PATTERN = true;
        }
		else if (strcmp(input,"-slicecustom") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicecustom !\n");
                return EXIT_FAILURE;
			}

			SLICE_ORDER = CUSTOM;
			SLICE_TIMINGS_FILE = argv[i+1];

            i += 2;
			DEFINED_SLICE_PATTERN = true;
        }
        else if (strcmp(input,"-slicecustomref") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -slicecustomref !\n");
                return EXIT_FAILURE;
			}

            SLICE_CUSTOM_REF = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Reference slice must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (SLICE_CUSTOM_REF < 0)
            {
                printf("Reference slice must be >= 0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
			DEFINED_SLICE_CUSTOM_REF = true;
        }
        else if (strcmp(input,"-iterationsmc") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -iterationsmc !\n");
                return EXIT_FAILURE;
			}
            
            NUMBER_OF_ITERATIONS_FOR_MOTION_CORRECTION = (int)strtol(argv[i+1], &p, 10);
            
			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of iterations for motion correction must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (NUMBER_OF_ITERATIONS_FOR_MOTION_CORRECTION < 0)
            {
                printf("Number of iterations for motion correction must be >= 0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-smoothing") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -smoothing !\n");
                return EXIT_FAILURE;
			}
            
            EPI_SMOOTHING_AMOUNT = (float)strtod(argv[i+1], &p);
            
			if (!isspace(*p) && *p != 0)
		    {
		        printf("Smoothing must be a float! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
  			else if ( EPI_SMOOTHING_AMOUNT <= 0.0f )
            {
                printf("Smoothing must be > 0.0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        
        // Statistical options

        else if (strcmp(input,"-detrendingregressors") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -detrendingregressors !\n");
                return EXIT_FAILURE;
			}

            NUMBER_OF_DETRENDING_REGRESSORS = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of detrending regressors must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            if ((NUMBER_OF_DETRENDING_REGRESSORS < 1) || (NUMBER_OF_DETRENDING_REGRESSORS > 4))
            {
                printf("Number of detrending regressors must be >= 1 & <= 4!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-betasonly") == 0)
        {
            BETAS_ONLY = true;
            i += 1;
        }
        else if (strcmp(input,"-rawregressors") == 0)
        {
            RAW_REGRESSORS = true;
            i += 1;
        }
        else if (strcmp(input,"-rawdesignmatrix") == 0)
        {
            RAW_DESIGNMATRIX = true;
            i += 1;
        }
        else if (strcmp(input,"-regressmotion") == 0)
        {
            REGRESS_MOTION = 1;
            i += 1;
        }
        else if (strcmp(input,"-regressglobalmean") == 0)
        {
            REGRESS_GLOBALMEAN = 1;
            i += 1;
        }
        else if (strcmp(input,"-temporalderivatives") == 0)
        {
            USE_TEMPORAL_DERIVATIVES = 1;
            i += 1;
        }
        else if (strcmp(input,"-permute") == 0)
        {
            PERMUTE = true;
            i += 1;

			printf("Permutation testing is currently turned off!\n");
            return EXIT_FAILURE;
        }
        else if (strcmp(input,"-permutations") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -permutations !\n");
                return EXIT_FAILURE;
			}

            NUMBER_OF_PERMUTATIONS = (int)strtol(argv[i+1], &p, 10);

			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of permutations must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (NUMBER_OF_PERMUTATIONS <= 0)
            {
                printf("Number of permutations must be > 0!\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-inferencemode") == 0)
        {
            if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -inferencemode !\n");
                return EXIT_FAILURE;
			}
            
            INFERENCE_MODE = (int)strtol(argv[i+1], &p, 10);
            
			if (!isspace(*p) && *p != 0)
		    {
		        printf("Inference mode must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if ( (INFERENCE_MODE != 0) && (INFERENCE_MODE != 1) && (INFERENCE_MODE != 2) && (INFERENCE_MODE != 3) )
            {
                printf("Inference mode must be 0, 1, 2 or 3 !\n");
                return EXIT_FAILURE;
            }
            i += 2;

			if (INFERENCE_MODE == 3)
			{
				printf("TFCE is currently turned off!\n");
    	        return EXIT_FAILURE;
			}
        }
        else if (strcmp(input,"-cdt") == 0)
        {
            if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -cdt !\n");
                return EXIT_FAILURE;
			}
            
            CLUSTER_DEFINING_THRESHOLD = (float)strtod(argv[i+1], &p);
            
			if (!isspace(*p) && *p != 0)
		    {
		        printf("Cluster defining threshold must be a float! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            i += 2;
        }
        else if (strcmp(input,"-bayesian") == 0)
        {
            BAYESIAN = true;
            i += 1;
        }
        else if (strcmp(input,"-iterationsmcmc") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read value after -iterationsmcmc !\n");
                return EXIT_FAILURE;
			}
            
            NUMBER_OF_MCMC_ITERATIONS = (int)strtol(argv[i+1], &p, 10);
            
			if (!isspace(*p) && *p != 0)
		    {
		        printf("Number of iterations for MCMC must be an integer! You provided %s \n",argv[i+1]);
				return EXIT_FAILURE;
		    }
            else if (NUMBER_OF_MCMC_ITERATIONS <= 0)
            {
                printf("Number of iterations for MCMC must be > 0 !\n");
                return EXIT_FAILURE;
            }
            i += 2;
        }
        else if (strcmp(input,"-mask") == 0)
        {
			if ( (i+1) >= argc  )
			{
			    printf("Unable to read name after -mask !\n");
                return EXIT_FAILURE;
			}
            
			MASK = true;
            MASK_NAME = argv[i+1];
            i += 2;
        }
        
        // Misc options
        else if (strcmp(input,"-savecompact") == 0)
        {
            WRITE_COMPACT = true;
            i += 1;
        }
        else if (strcmp(input,"-savetransformations") == 0)
        {
            WRITE_TRANSFORMATION_MATRICES = true;
            i += 1;
        }
        else if (strcmp(input,"-savet1interpolated") == 0)
        {
            WRITE_INTERPOLATED_T1 = true;
            i += 1;
        }
        else if (strcmp(input,"-savet1alignedlinear") == 0)
        {
            WRITE_ALIGNED_T1_MNI_LINEAR = true;
            i += 1;
        }
        else if (strcmp(input,"-savet1alignednonlinear") == 0)
        {
            WRITE_ALIGNED_T1_MNI_NONLINEAR = true;
            i += 1;
        }
        else if (strcmp(input,"-saveepialignedt1") == 0)
        {
            WRITE_ALIGNED_EPI_T1 = true;
            i += 1;
        }
        else if (strcmp(input,"-saveepialignedmni") == 0)
        {
            WRITE_ALIGNED_EPI_MNI = true;
            i += 1;
        }
        else if (strcmp(input,"-saveepimask") == 0)
        {
            WRITE_EPI_MASK = true;
            i += 1;
        }
        else if (strcmp(input,"-savemnimask") == 0)
        {
            WRITE_MNI_MASK = true;
            i += 1;
        }
        else if (strcmp(input,"-saveslicetimingcorrected") == 0)
        {
            WRITE_SLICETIMING_CORRECTED = true;
            i += 1;
        }
        else if (strcmp(input,"-savemotioncorrected") == 0)
        {
            WRITE_MOTION_CORRECTED = true;
            i += 1;
        }
        else if (strcmp(input,"-savemotionparameters") == 0)
        {
            WRITE_MOTION_PARAMETERS = true;
            i += 1;
        }
        else if (strcmp(input,"-savesmoothed") == 0)
        {
            WRITE_SMOOTHED = true;
            i += 1;
        }
        else if (strcmp(input,"-saveactivityepi") == 0)
        {
            WRITE_ACTIVITY_EPI = true;
            i += 1;
        }
        else if (strcmp(input,"-saveactivityt1") == 0)
        {
            WRITE_ACTIVITY_T1 = true;
            i += 1;
        }
        else if (strcmp(input,"-saveresiduals") == 0)
        {
            WRITE_RESIDUALS_EPI = true;
            i += 1;
        }
        else if (strcmp(input,"-saveresidualsmni") == 0)
        {
            WRITE_RESIDUALS_MNI = true;
            i += 1;
			printf("Saving residuals to MNI space is currently turned off!\n");
    	    return EXIT_FAILURE;
        }
        else if (strcmp(input,"-savedesignmatrix") == 0)
        {
            WRITE_DESIGNMATRIX = true;
            i += 1;
        }
        else if (strcmp(input,"-saveoriginaldesignmatrix") == 0)
        {
            WRITE_ORIGINAL_DESIGNMATRIX = true;
            i += 1;
        }
        else if (strcmp(input,"-savearparameters") == 0)
        {
            WRITE_AR_ESTIMATES_EPI = true;
            i += 1;
        }
        else if (strcmp(input,"-savearparameterst1") == 0)
        {
            WRITE_AR_ESTIMATES_T1 = true;
            i += 1;
        }
        else if (strcmp(input,"-savearparametersmni") == 0)
        {
            WRITE_AR_ESTIMATES_MNI = true;
            i += 1;
        }
        else if (strcmp(input,"-saveallaligned") == 0)
        {
            WRITE_INTERPOLATED_T1 = true;
            WRITE_ALIGNED_T1_MNI_LINEAR = true;
            WRITE_ALIGNED_T1_MNI_NONLINEAR = true;
            WRITE_ALIGNED_EPI_T1 = true;
            WRITE_ALIGNED_EPI_MNI = true;
            i += 1;
        }
        else if (strcmp(input,"-saveallpreprocessed") == 0)
        {
            WRITE_SLICETIMING_CORRECTED = true;
            WRITE_MOTION_CORRECTED = true;
            WRITE_SMOOTHED = true;
			i += 1;
		}
        else if (strcmp(input,"-saveunwhitenedresults") == 0)
        {
            WRITE_UNWHITENED_RESULTS = true;
			i += 1;
		}
        else if (strcmp(input,"-saveall") == 0)
        {
            WRITE_INTERPOLATED_T1 = true;
            WRITE_ALIGNED_T1_MNI_LINEAR = true;
            WRITE_ALIGNED_T1_MNI_NONLINEAR = true;
            WRITE_ALIGNED_EPI_T1 = true;
            WRITE_ALIGNED_EPI_MNI = true;
			WRITE_EPI_MASK = true;
			WRITE_MNI_MASK = true;
            WRITE_SLICETIMING_CORRECTED = true;
            WRITE_MOTION_CORRECTED = true;
            WRITE_SMOOTHED = true;
            WRITE_ACTIVITY_EPI = true;
            WRITE_RESIDUALS_EPI = true;
            WRITE_RESIDUALS_MNI = true;
            WRITE_AR_ESTIMATES_EPI = true;
            WRITE_AR_ESTIMATES_MNI = true;
			WRITE_UNWHITENED_RESULTS = true;
			WRITE_MOTION_CORRECTED = true;
            i += 1;
        }
        else if (strcmp(input,"-quiet") == 0)
        {
            PRINT = false;
            i += 1;
        }
        else if (strcmp(input,"-verbose") == 0)
        {
            VERBOS = true;
            i += 1;
        }
        else if (strcmp(input,"-debug") == 0)
        {
            DEBUG = true;
            i += 1;
        }
        else if (strcmp(input,"-output") == 0)
        {
			CHANGE_OUTPUT_FILENAME = true;

			if ( (i+1) >= argc  )
			{
			    printf("Unable to read name after -output !\n");
                return EXIT_FAILURE;
			}

            outputFilename = argv[i+1];
            i += 2;
        }
        else
        {
            printf("Unrecognized option! %s \n",argv[i]);
            return EXIT_FAILURE;
        }                
    }
    
	// Check if BROCCOLI_DIR variable is set
	if (getenv("BROCCOLI_DIR") == NULL)
	{
        printf("The environment variable BROCCOLI_DIR is not set!\n");
        return EXIT_FAILURE;
	}

    //------------------------------------------
	// Check for invalid parameter combinations

    if (BAYESIAN && PERMUTE)
    {
        printf("Cannot do both Bayesian and non-parametric fMRI analysis, pick one!\n");
        return EXIT_FAILURE;
    }
	if (WRITE_UNWHITENED_RESULTS && (REGRESS_ONLY || PREPROCESSING_ONLY))
	{
        printf("Cannot write unwhitened resuls if you only do regression or preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (!APPLY_MOTION_CORRECTION && REGRESS_MOTION)
	{
		printf("Nice try! Cannot regress motion if you skip motion correction!\n");
		return EXIT_FAILURE;
	}
	if (!APPLY_MOTION_CORRECTION && WRITE_MOTION_PARAMETERS)
	{
		printf("Nice try! Cannot save motion parameters if you skip motion correction!\n");
		return EXIT_FAILURE;
	}
	if (RAW_DESIGNMATRIX && USE_TEMPORAL_DERIVATIVES)
	{
		printf("Cannot use temporal derivatives for raw design matrix!\n");
		return EXIT_FAILURE;
	}
	if (!APPLY_MOTION_CORRECTION && WRITE_MOTION_CORRECTED)
	{
		printf("Nice try! Cannot save motion corrected data if you skip motion correction!\n");
		return EXIT_FAILURE;
	}
	if (!APPLY_SMOOTHING && WRITE_SMOOTHED)
	{
		printf("Nice try! Cannot save smoothed data if you skip smoothing!\n");
		return EXIT_FAILURE;
	}
	if (!APPLY_SLICE_TIMING_CORRECTION && WRITE_SLICETIMING_CORRECTED)
	{
		printf("Nice try! Cannot save slice timing corrected data if you skip slice timing correction!\n");
		return EXIT_FAILURE;
	}
	if ((WRITE_AR_ESTIMATES_EPI || WRITE_AR_ESTIMATES_T1 || WRITE_AR_ESTIMATES_MNI) && (REGRESS_ONLY || PREPROCESSING_ONLY))
	{
        printf("Cannot write AR parameters if you only do regression or preprocessing!\n");
        return EXIT_FAILURE;
	}
	if ((WRITE_RESIDUALS_EPI || WRITE_RESIDUALS_MNI) && PREPROCESSING_ONLY)
	{
        printf("Cannot write residuals if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (REGRESS_GLOBALMEAN && PREPROCESSING_ONLY)
	{
        printf("Cannot regress global mean if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (REGRESS_MOTION && PREPROCESSING_ONLY)
	{
        printf("Cannot regress global mean if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (RAW_REGRESSORS && PREPROCESSING_ONLY)
	{
        printf("Raw regressors does not have any meaning if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (RAW_DESIGNMATRIX && PREPROCESSING_ONLY)
	{
        printf("Raw design matrix does not have any meaning if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (PERMUTE && PREPROCESSING_ONLY)
	{
        printf("Permute does not have any meaning if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (BAYESIAN && PREPROCESSING_ONLY)
	{
        printf("Bayesian does not have any meaning if you only do preprocessing!\n");
        return EXIT_FAILURE;
	}
	if (PERMUTE && REGRESS_ONLY)
	{
        printf("Permute does not have any meaning if you only do regression!\n");
        return EXIT_FAILURE;
	}
	if (BAYESIAN && REGRESS_ONLY)
	{
        printf("Bayesian does not have any meaning if you only do regression!\n");
        return EXIT_FAILURE;
	}
	if (PERMUTE && BETAS_ONLY)
	{
        printf("Permute does not have any meaning if you only calculate betas and contrasts!\n");
        return EXIT_FAILURE;
	}


    //------------------------------------------

    // Read number of regressors from design matrix file
  
	std::ifstream design;
    std::string tempString;
    int tempNumber;
    std::string NR("NumRegressors");

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
		int argument;

		if (!MULTIPLE_RUNS)
		{
			argument = 4;
		}
		else
		{
			argument = 5 + NUMBER_OF_RUNS;
		}

	    design.open(argv[argument]);
    
	    if (!design.good())
	    {
	        design.close();
	        printf("Unable to open design file %s. Aborting! \n",argv[argument]);
	        return EXIT_FAILURE;
	    }
    
	    // Get number of regressors
	    design >> tempString; // NumRegressors as string
	    if (tempString.compare(NR) != 0)
	    {
	        design.close();
	        printf("First element of the design file %s should be the string 'NumRegressors', but it is %s. Aborting! \n",argv[argument],tempString.c_str());
	        return EXIT_FAILURE;
	    }
	    design >> NUMBER_OF_GLM_REGRESSORS;
    
	    if (NUMBER_OF_GLM_REGRESSORS <= 0)
	    {
	        design.close();
	        printf("Number of regressors must be > 0 ! You provided %zu regressors in the design file %s. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,argv[argument]);
	        return EXIT_FAILURE;
	    }
	    else if ((NUMBER_OF_GLM_REGRESSORS > 25) && PERMUTE)
	    {
	        design.close();
	        printf("Number of regressors must be <= 25 when permuting ! You provided %zu regressors in the design file %s. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,argv[argument]);
	        return EXIT_FAILURE;
	    }
	    else if ( BAYESIAN && (NUMBER_OF_GLM_REGRESSORS != 2) )
	    {
	        design.close();
	        printf("Number of regressors must currently be exactly 2 for Bayesian fMRI analysis! You provided %zu regressors in the design file %s. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,argv[argument]);
	        return EXIT_FAILURE;
	    }
	    design.close();
	}
	else
	{
		NUMBER_OF_GLM_REGRESSORS = 0;
	}
  
    // Read contrasts
   	std::ifstream contrasts;    

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
		if (!BAYESIAN)
		{
		int argument;

			if (!MULTIPLE_RUNS)
			{
				argument = 5;
			}
			else
			{
				argument = 5 + NUMBER_OF_RUNS*2;
			}

		    contrasts.open(argv[argument]);
    
		    if (!contrasts.good())
		    {
		        contrasts.close();
		        printf("Unable to open contrasts file %s. Aborting! \n",argv[argument]);
		        return EXIT_FAILURE;
		    }
    
		    contrasts >> tempString; // NumRegressors as string
		    if (tempString.compare(NR) != 0)
		    {
		        contrasts.close();
		        printf("First element of the contrasts file should be the string 'NumRegressors', but it is %s. Aborting! \n",tempString.c_str());
		        return EXIT_FAILURE;
		    }
		    contrasts >> tempNumber;
    
		    // Check for consistency
		    if ( tempNumber != NUMBER_OF_GLM_REGRESSORS )
    		{
		        contrasts.close();
		        printf("Design file says that number of regressors is %zu, while contrast file says there are %i regressors. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,tempNumber);
		        return EXIT_FAILURE;
		    }
    
		    contrasts >> tempString; // NumContrasts as string
		    std::string NC("NumContrasts");
		    if (tempString.compare(NC) != 0)
		    {
		        contrasts.close();
		        printf("Third element of the contrasts file should be the string 'NumContrasts', but it is %s. Aborting! \n",tempString.c_str());
		        return EXIT_FAILURE;
		    }
		    contrasts >> NUMBER_OF_CONTRASTS;
			
		    if (NUMBER_OF_CONTRASTS <= 0)
		    {
		        contrasts.close();
    		    printf("Number of contrasts must be > 0 ! You provided %zu in the contrasts file. Aborting! \n",NUMBER_OF_CONTRASTS);
		        return EXIT_FAILURE;
		    }
		    contrasts.close();
		}
		else if (BAYESIAN)
		{
			NUMBER_OF_CONTRASTS = 6;
    	    printf("Warning: Your contrasts are not used for the Bayesian fMRI analysis. Since only 2 regressors are currently supported, all the 6 possible contrasts are always calculated\n");
		}
    }
	else
	{
		NUMBER_OF_CONTRASTS = 0;
	}
    
	//------------------------------------------

	double startTime = GetWallTime();

	// -----------------------    
    // Read fMRI data
	// -----------------------
	nifti_image *inputfMRI;
	std::vector<nifti_image*> allfMRINiftiImages;

	if (!MULTIPLE_RUNS)
	{
		inputfMRI = nifti_image_read(argv[1],1);
	    allfMRINiftiImages.push_back(inputfMRI);

    	if (inputfMRI == NULL)
    	{
    	    printf("Could not open fMRI data!\n");
    	    return EXIT_FAILURE;
    	}
		allNiftiImages[numberOfNiftiImages] = inputfMRI;
		numberOfNiftiImages++;
	}
	else
	{
		for (int i = 0; i < NUMBER_OF_RUNS; i++)
		{
			inputfMRI = nifti_image_read(argv[3+i],1);
			allfMRINiftiImages.push_back(inputfMRI);    

    		if (inputfMRI == NULL)
    		{
    		    printf("Could not open fMRI data for run %i !\n",i+1);
    		    return EXIT_FAILURE;
    		}
			allNiftiImages[numberOfNiftiImages] = inputfMRI;
			numberOfNiftiImages++;
		    EPI_DATA_T_PER_RUN[i] = inputfMRI->nt;    
		}
	}

	// -----------------------    
    // Read T1 volume
	// -----------------------
	nifti_image *inputT1;

	if (!MULTIPLE_RUNS)
	{
		inputT1 = nifti_image_read(argv[2],1);
	}
	else
	{
		inputT1 = nifti_image_read(argv[3+NUMBER_OF_RUNS],1);
	}
    
    if (inputT1 == NULL)
    {
        printf("Could not open T1 volume!\n");
		FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }
	allNiftiImages[numberOfNiftiImages] = inputT1;
	numberOfNiftiImages++;

	// -----------------------    
    // Read brain template
	// -----------------------
	nifti_image *inputMNI;

	if (!MULTIPLE_RUNS)
	{
		inputMNI = nifti_image_read(argv[3],1);
	}
	else
	{
		inputMNI = nifti_image_read(argv[4+NUMBER_OF_RUNS],1);
	}
    
    if (inputMNI == NULL)
    {
        printf("Could not open MNI volume!\n");
		FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }
	allNiftiImages[numberOfNiftiImages] = inputMNI;
	numberOfNiftiImages++;
    
	// -----------------------    
    // Read mask
	// -----------------------

    nifti_image *inputMask;
    if (MASK)
    {
        inputMask = nifti_image_read(MASK_NAME,1);
        if (inputMask == NULL)
        {
            printf("Could not open mask volume!\n");
            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
            return EXIT_FAILURE;
        }
        allNiftiImages[numberOfNiftiImages] = inputMask;
        numberOfNiftiImages++;
    }

	double endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to read the nifti file(s)\n",(float)(endTime - startTime));
	}

	// -----------------------    
    
    // Get fMRI data dimensions
    EPI_DATA_W = inputfMRI->nx;
    EPI_DATA_H = inputfMRI->ny;
    EPI_DATA_D = inputfMRI->nz; 

	if (!MULTIPLE_RUNS)
	{   
	    EPI_DATA_T = inputfMRI->nt;   
		EPI_DATA_T_PER_RUN[0] = EPI_DATA_T; 
	}
	else
	{
		EPI_DATA_T = 0;
		for (int i = 0; i < NUMBER_OF_RUNS; i++)
		{
		    EPI_DATA_T += EPI_DATA_T_PER_RUN[i];
		}
	}

    // Get fMRI voxel sizes
    EPI_VOXEL_SIZE_X = inputfMRI->dx;
    EPI_VOXEL_SIZE_Y = inputfMRI->dy;
    EPI_VOXEL_SIZE_Z = inputfMRI->dz;

    // Get fMRI repetition time
    TR = inputfMRI->dt;

	if (DEFINED_SLICE_CUSTOM_REF)
	{
	    if (SLICE_CUSTOM_REF >= EPI_DATA_D) 
	    {
	    	printf("Reference slice must be < number of slices!\n");
			FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	        return EXIT_FAILURE;
	    }
	}
	else
	{
		SLICE_CUSTOM_REF = (int)round((float)EPI_DATA_D/2.0f);
	}

	// Get fMRI slice order
	int SLICE_ORDER_NIFTI = (int)inputfMRI->slice_code;

	std::string SLICE_ORDER_STRING;

	// No slice pattern given by user, so use the one from the nifti file (if not unknown)
	if (!DEFINED_SLICE_PATTERN)
	{
		if (SLICE_ORDER_NIFTI == NIFTI_SLICE_SEQ_INC)
		{
			SLICE_ORDER_STRING = std::string("Seqential increasing");
			SLICE_ORDER = UP;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_SEQ_DEC)
		{
			SLICE_ORDER_STRING = std::string("Seqential decreasing");
			SLICE_ORDER = DOWN;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_INC)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing");
			SLICE_ORDER = UP_INTERLEAVED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_DEC)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing");
			SLICE_ORDER = DOWN_INTERLEAVED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_INC2)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing 2, not yet supported. Use -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
		else if (SLICE_ORDER_NIFTI == NIFTI_SLICE_ALT_DEC2)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing 2, not yet supported. Use -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
		else
		{
			SLICE_ORDER_STRING = std::string("Unknown, need to specify with option -slicepattern or -slicecustom");
			SLICE_ORDER = UNDEFINED;
		}
	}
	// Slice pattern defined by user
	else
	{
		if (SLICE_ORDER == UP)
		{
			SLICE_ORDER_STRING = std::string("Seqential increasing");
		}
		else if (SLICE_ORDER == DOWN)
		{
			SLICE_ORDER_STRING = std::string("Seqential decreasing");
		}
		else if (SLICE_ORDER == UP_INTERLEAVED)
		{
			SLICE_ORDER_STRING = std::string("Alternating increasing");
		}
		else if (SLICE_ORDER == DOWN_INTERLEAVED)
		{
			SLICE_ORDER_STRING = std::string("Alternating decreasing");
		}
		else if (SLICE_ORDER == CUSTOM)
		{
			SLICE_ORDER_STRING = std::string("Custom slice order defined by file");
		}
	}

    //------------------------------------------	
	// Read slice timing information from text file
	
	if (SLICE_ORDER == CUSTOM)
	{		
		std::ifstream slicetimes;
		slicetimes.open(SLICE_TIMINGS_FILE);

		if (!slicetimes.good())    
		{
			slicetimes.close();
	        printf("Unable to open slice timing file %s. Aborting! \n",SLICE_TIMINGS_FILE);
			FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	        return EXIT_FAILURE;
		}

		// Loop over slices
	    for (int slice = 0; slice < EPI_DATA_D; slice++)
	    {
	        float time;
    	        
	        // Read onset, duration and value for current event
			if (! (slicetimes >> time) )
			{
	            slicetimes.close();
	            printf("Unable to read the slice time for slice %i in slice timing file %s, aborting! Check the slice timing file. \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}

			if (time > (TR*1000.0f))
			{
	            slicetimes.close();
	            printf("Slice time cannot be larger than the TR! Check the time for slice %i in slice timing file %s ! \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}

			if (time < 0.0f)
			{
	            slicetimes.close();
	            printf("Slice time cannot be negative! Check the time for slice %i in slice timing file %s ! \n",slice,SLICE_TIMINGS_FILE);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
	            return EXIT_FAILURE;
			}
	
			h_Custom_Slice_Times[slice] = time/1000.0f;		

			if (DEBUG)
			{
				printf("Slice time for slice %i is %f milli seconds \n",slice,time);
			}
		}
		slicetimes.close();
	}

    //------------------------------------------	

	// Get T1 data dimensions
    T1_DATA_W = inputT1->nx;
    T1_DATA_H = inputT1->ny;
    T1_DATA_D = inputT1->nz;    

	// Get T1 voxel sizes
    T1_VOXEL_SIZE_X = inputT1->dx;
    T1_VOXEL_SIZE_Y = inputT1->dy;
    T1_VOXEL_SIZE_Z = inputT1->dz;
    
	// Get brain template data dimensions
    MNI_DATA_W = inputMNI->nx;
    MNI_DATA_H = inputMNI->ny;
    MNI_DATA_D = inputMNI->nz;    
    
	// Get brain template voxel sizes                             
    MNI_VOXEL_SIZE_X = inputMNI->dx;
    MNI_VOXEL_SIZE_Y = inputMNI->dy;
    MNI_VOXEL_SIZE_Z = inputMNI->dz;

	// The filter size is 7, so select a lowest scale for the registrations that gives at least 10 valid samples (3 data points are lost on each side in each dimension, i.e. 6 total)
	if ( (MNI_DATA_W/16 >= 16) && (MNI_DATA_H/16 >= 16) && (MNI_DATA_D/16 >= 16) )
	{
		COARSEST_SCALE_T1_MNI = 16;
		COARSEST_SCALE_EPI_T1 = 16;
	}
	else if ( (MNI_DATA_W/8 >= 16) && (MNI_DATA_H/8 >= 16) && (MNI_DATA_D/8 >= 16) )
	{
		COARSEST_SCALE_T1_MNI = 8;
		COARSEST_SCALE_EPI_T1 = 8;
	}
	else if ( (MNI_DATA_W/4 >= 16) && (MNI_DATA_H/4 >= 16) && (MNI_DATA_D/4 >= 16) )
	{
		COARSEST_SCALE_T1_MNI = 4;
		COARSEST_SCALE_EPI_T1 = 4;
	}
	else if ( (MNI_DATA_W/2 >= 16) && (MNI_DATA_H/2 >= 16) && (MNI_DATA_D/2 >= 16) )
	{
		COARSEST_SCALE_T1_MNI = 2;
		COARSEST_SCALE_EPI_T1 = 2;
	}
	else
	{
		COARSEST_SCALE_T1_MNI = 1;
		COARSEST_SCALE_EPI_T1 = 1;
	}

    // Calculate sizes, in bytes
    NUMBER_OF_TOTAL_GLM_REGRESSORS = 4*NUMBER_OF_RUNS;
	if (!BAYESIAN && !RAW_DESIGNMATRIX)
	{
		NUMBER_OF_TOTAL_GLM_REGRESSORS = NUMBER_OF_GLM_REGRESSORS * (USE_TEMPORAL_DERIVATIVES+1) + NUMBER_OF_DETRENDING_REGRESSORS*NUMBER_OF_RUNS + NUMBER_OF_MOTION_REGRESSORS * REGRESS_MOTION + REGRESS_GLOBALMEAN; //NUMBER_OF_CONFOUND_REGRESSORS*REGRESS_CONFOUNDS;
	}
	else if (RAW_DESIGNMATRIX)
	{
		NUMBER_OF_TOTAL_GLM_REGRESSORS = NUMBER_OF_GLM_REGRESSORS + NUMBER_OF_DETRENDING_REGRESSORS*NUMBER_OF_RUNS + NUMBER_OF_MOTION_REGRESSORS * REGRESS_MOTION + REGRESS_GLOBALMEAN; //NUMBER_OF_CONFOUND_REGRESSORS*REGRESS_CONFOUNDS;
	}
	else if (BAYESIAN)
	{
		NUMBER_OF_TOTAL_GLM_REGRESSORS = 2;
	}
    
    if ((NUMBER_OF_TOTAL_GLM_REGRESSORS > 25) && PERMUTE)
    {
        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
		if (REGRESS_MOTION)
		{
        	printf("Number of total regressors must be <= 25 when permuting ! You provided %zu regressors in the design file, with 6 regressors for motion and 4 for detrending, this comes to a total of %zu regressors. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,NUMBER_OF_TOTAL_GLM_REGRESSORS);
		}
		else
		{
        	printf("Number of total regressors must be <= 25 when permuting ! You provided %zu regressors in the design file, with 4 regressors for detrending, this comes to a total of %zu regressors. Aborting! \n",NUMBER_OF_GLM_REGRESSORS,NUMBER_OF_TOTAL_GLM_REGRESSORS);
		}
        return EXIT_FAILURE;
    }

    size_t EPI_DATA_SIZE = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T * sizeof(float);
    size_t T1_VOLUME_SIZE = T1_DATA_W * T1_DATA_H * T1_DATA_D * sizeof(float);
    size_t MNI_VOLUME_SIZE = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * sizeof(float);
    
    size_t EPI_VOLUME_SIZE = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * sizeof(float);
    //size_t EPI_VOLUME_SIZE_INT = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * sizeof(int);
    
    size_t FILTER_SIZE = IMAGE_REGISTRATION_FILTER_SIZE * IMAGE_REGISTRATION_FILTER_SIZE * IMAGE_REGISTRATION_FILTER_SIZE * sizeof(float);
    
    size_t MOTION_PARAMETERS_SIZE = NUMBER_OF_IMAGE_REGISTRATION_PARAMETERS_RIGID * EPI_DATA_T * sizeof(float);
    
    size_t GLM_SIZE = EPI_DATA_T * NUMBER_OF_GLM_REGRESSORS * sizeof(float);
    size_t CONTRAST_SIZE = NUMBER_OF_GLM_REGRESSORS * NUMBER_OF_CONTRASTS * sizeof(float);
    size_t CONTRAST_SCALAR_SIZE = NUMBER_OF_CONTRASTS * sizeof(float);
    size_t DESIGN_MATRIX_SIZE = NUMBER_OF_TOTAL_GLM_REGRESSORS * EPI_DATA_T * sizeof(float);
	size_t HIGHRES_REGRESSORS_SIZE = NUMBER_OF_GLM_REGRESSORS * EPI_DATA_T * HIGHRES_FACTOR * sizeof(float);    

    //size_t CONFOUNDS_SIZE = NUMBER_OF_CONFOUND_REGRESSORS * EPI_DATA_T * sizeof(float);
    
    size_t PROJECTION_TENSOR_SIZE = NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION * sizeof(float);
    size_t FILTER_DIRECTIONS_SIZE = NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION * sizeof(float);
    
    size_t BETA_DATA_SIZE_MNI = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS * sizeof(float);
    size_t STATISTICAL_MAPS_DATA_SIZE_MNI = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_CONTRASTS * sizeof(float);
    size_t RESIDUALS_DATA_SIZE_MNI = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * EPI_DATA_T * sizeof(float);
 
    size_t BETA_DATA_SIZE_EPI = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS * sizeof(float);
    size_t STATISTICAL_MAPS_DATA_SIZE_EPI = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_CONTRASTS * sizeof(float);
    size_t RESIDUALS_DATA_SIZE_EPI = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T * sizeof(float);

    size_t BETA_DATA_SIZE_T1 = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS * sizeof(float);
    size_t STATISTICAL_MAPS_DATA_SIZE_T1 = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_CONTRASTS * sizeof(float);
    //size_t RESIDUALS_DATA_SIZE_T1 = T1_DATA_W * T1_DATA_H * T1_DATA_D * EPI_DATA_T * sizeof(float);
    
	size_t PERMUTATION_MATRIX_SIZE = NUMBER_OF_PERMUTATIONS * EPI_DATA_T * sizeof(unsigned short int);
	size_t NULL_DISTRIBUTION_SIZE = NUMBER_OF_PERMUTATIONS * NUMBER_OF_CONTRASTS * sizeof(float);

    // Print some info
    if (PRINT)
    {
        printf("\nAuthored by K.A. Eklund \n");
		if (!MULTIPLE_RUNS)
		{
		    printf("fMRI data size: %zu x %zu x %zu x %zu \n", EPI_DATA_W, EPI_DATA_H, EPI_DATA_D, EPI_DATA_T);
		}
		else
		{
			for (int i = 0; i < NUMBER_OF_RUNS; i++)
			{
			    printf("fMRI data size for run %i: %zu x %zu x %zu x %zu \n", i+1, EPI_DATA_W, EPI_DATA_H, EPI_DATA_D, EPI_DATA_T_PER_RUN[i]);
			}
		    printf("fMRI total data size: %zu x %zu x %zu x %zu \n", EPI_DATA_W, EPI_DATA_H, EPI_DATA_D, EPI_DATA_T);
		}
		printf("fMRI voxel size: %f x %f x %f mm \n", EPI_VOXEL_SIZE_X, EPI_VOXEL_SIZE_Y, EPI_VOXEL_SIZE_Z);
		printf("fMRI TR: %f s \n", TR);		
		printf("fMRI slice order: %s \n",SLICE_ORDER_STRING.c_str());
    	printf("T1 data size: %zu x %zu x %zu \n", T1_DATA_W, T1_DATA_H, T1_DATA_D);
		printf("T1 voxel size: %f x %f x %f mm \n", T1_VOXEL_SIZE_X, T1_VOXEL_SIZE_Y, T1_VOXEL_SIZE_Z);
	    printf("MNI data size: %zu x %zu x %zu \n", MNI_DATA_W, MNI_DATA_H, MNI_DATA_D);
		printf("MNI voxel size: %f x %f x %f mm \n", MNI_VOXEL_SIZE_X, MNI_VOXEL_SIZE_Y, MNI_VOXEL_SIZE_Z);
		if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
		{
		    printf("Number of original GLM regressors: %zu \n",  NUMBER_OF_GLM_REGRESSORS);
		}
		if (!PREPROCESSING_ONLY)
		{
	  	    printf("Number of total GLM regressors: %zu \n",  NUMBER_OF_TOTAL_GLM_REGRESSORS);
		}
		if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
		{
		    printf("Number of contrasts: %zu \n",  NUMBER_OF_CONTRASTS);
		}
    } 
   	if (VERBOS)
 	{
		printf("Selected lowest scale %i for the registration \n",COARSEST_SCALE_T1_MNI);
	}

    // ------------------------------------------------
    
    // Allocate memory on the host

	startTime = GetWallTime();

	if (!MULTIPLE_RUNS)
	{
		// If the data is in float format, we can just copy the pointer
		if ( inputfMRI->datatype != DT_FLOAT )
		{
			AllocateMemory(h_fMRI_Volumes, EPI_DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "fMRI_VOLUMES");
		}
		else
		{
			allocatedHostMemory += EPI_DATA_SIZE;
		}
	}
	else
	{
		AllocateMemory(h_fMRI_Volumes, EPI_DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "fMRI_VOLUMES");
	}
	AllocateMemory(h_T1_Volume, T1_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "T1_VOLUME");
	AllocateMemory(h_MNI_Volume, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "MNI_VOLUME");
	AllocateMemory(h_MNI_Brain_Volume, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "MNI_BRAIN_VOLUME");
 
    if (WRITE_INTERPOLATED_T1)
    {
        AllocateMemory(h_Interpolated_T1_Volume, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "T1_INTERPOLATED");
    }
    if (WRITE_ALIGNED_T1_MNI_LINEAR)
    {
        AllocateMemory(h_Aligned_T1_Volume_Linear, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "ALIGNED_T1_LINEAR");
    }
    if (WRITE_ALIGNED_T1_MNI_NONLINEAR)
    {
        AllocateMemory(h_Aligned_T1_Volume_NonLinear, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "ALIGNED_T1_NONLINEAR");
    }
	if (WRITE_ALIGNED_EPI_T1)
	{
        AllocateMemory(h_Aligned_EPI_Volume_T1, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "ALIGNED_EPI_T1");
	}
	if (WRITE_ALIGNED_EPI_MNI)
	{
        AllocateMemory(h_Aligned_EPI_Volume_MNI_Linear, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "ALIGNED_EPI_MNI_Linear");
        AllocateMemory(h_Aligned_EPI_Volume_MNI_Nonlinear, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "ALIGNED_EPI_MNI_Nonlinear");
	}

	AllocateMemory(h_Quadrature_Filter_1_Linear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_1_LINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_1_Linear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_1_LINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_2_Linear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_2_LINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_2_Linear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_2_LINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_3_Linear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_3_LINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_3_Linear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_3_LINEAR_REGISTRATION_IMAG");    
    
	AllocateMemory(h_Quadrature_Filter_1_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_1_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_1_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_1_NONLINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_2_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_2_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_2_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_2_NONLINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_3_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_3_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_3_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_3_NONLINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_4_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_4_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_4_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_4_NONLINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_5_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_5_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_5_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_5_NONLINEAR_REGISTRATION_IMAG");    
	AllocateMemory(h_Quadrature_Filter_6_NonLinear_Registration_Real, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_6_NONLINEAR_REGISTRATION_REAL");    
	AllocateMemory(h_Quadrature_Filter_6_NonLinear_Registration_Imag, FILTER_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "QUADRATURE_FILTER_6_NONLINEAR_REGISTRATION_IMAG");    

    AllocateMemory(h_Projection_Tensor_1, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_1");    
    AllocateMemory(h_Projection_Tensor_2, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_2");    
    AllocateMemory(h_Projection_Tensor_3, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_3");    
    AllocateMemory(h_Projection_Tensor_4, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_4");    
    AllocateMemory(h_Projection_Tensor_5, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_5");    
    AllocateMemory(h_Projection_Tensor_6, PROJECTION_TENSOR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PROJECTION_TENSOR_6");    

    AllocateMemory(h_Filter_Directions_X, FILTER_DIRECTIONS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory,  "FILTER_DIRECTIONS_X");
    AllocateMemory(h_Filter_Directions_Y, FILTER_DIRECTIONS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "FILTER_DIRECTIONS_Y");        
    AllocateMemory(h_Filter_Directions_Z, FILTER_DIRECTIONS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "FILTER_DIRECTIONS_Z");                
   
	AllocateMemory(h_Motion_Parameters, MOTION_PARAMETERS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "MOTION_PARAMETERS");       

	if (WRITE_EPI_MASK || WRITE_MNI_MASK)
	{
		AllocateMemory(h_EPI_Mask, EPI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "EPI_MASK");
	}
	if (WRITE_MNI_MASK)
	{
		AllocateMemory(h_MNI_Mask, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "MNI_MASK");
	}
	if (WRITE_SLICETIMING_CORRECTED)
	{
		AllocateMemory(h_Slice_Timing_Corrected_fMRI_Volumes, EPI_DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "SLICETIMINGCORRECTED_fMRI_VOLUMES");
	}
	if (WRITE_MOTION_CORRECTED)
	{
		AllocateMemory(h_Motion_Corrected_fMRI_Volumes, EPI_DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "MOTIONCORRECTED_fMRI_VOLUMES");
	}
	if (WRITE_SMOOTHED)
	{
		AllocateMemory(h_Smoothed_fMRI_Volumes, EPI_DATA_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "SMOOTHED_fMRI_VOLUMES");
	}



	if (!REGRESS_ONLY && !BETAS_ONLY && !PREPROCESSING_ONLY)
	{
	    AllocateMemory(h_Beta_Volumes_MNI, BETA_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_MNI");
		AllocateMemory(h_Contrast_Volumes_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_MNI");
		AllocateMemory(h_Statistical_Maps_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_MNI");
    
		if (WRITE_UNWHITENED_RESULTS)
		{
		    AllocateMemory(h_Beta_Volumes_No_Whitening_MNI, BETA_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_MNI");
			AllocateMemory(h_Contrast_Volumes_No_Whitening_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_MNI");
			AllocateMemory(h_Statistical_Maps_No_Whitening_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_MNI");
		}

	    if (WRITE_ACTIVITY_EPI)
	    {
	        AllocateMemory(h_Beta_Volumes_EPI, BETA_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_EPI");
	        AllocateMemory(h_Contrast_Volumes_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_EPI");
	        AllocateMemory(h_Statistical_Maps_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_EPI");

			if (WRITE_UNWHITENED_RESULTS)
			{
	        	AllocateMemory(h_Beta_Volumes_No_Whitening_EPI, BETA_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_EPI");
		        AllocateMemory(h_Contrast_Volumes_No_Whitening_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_EPI");
		        AllocateMemory(h_Statistical_Maps_No_Whitening_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_EPI");
			}

			if (PERMUTE)
			{
				AllocateMemory(h_P_Values_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PVALUES_EPI");
			}
	    }        

	    if (WRITE_ACTIVITY_T1)
    	{
	        AllocateMemory(h_Beta_Volumes_T1, BETA_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_T1");
	        AllocateMemory(h_Contrast_Volumes_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_T1");
	        AllocateMemory(h_Statistical_Maps_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_T1");

			if (WRITE_UNWHITENED_RESULTS)
			{
	        	AllocateMemory(h_Beta_Volumes_No_Whitening_T1, BETA_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_T1");
		        AllocateMemory(h_Contrast_Volumes_No_Whitening_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_T1");
		        AllocateMemory(h_Statistical_Maps_No_Whitening_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "STATISTICALMAPS_T1");
			}

			if (PERMUTE)
			{
				AllocateMemory(h_P_Values_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PVALUES_T1");
			}
	    }        

		if (PERMUTE)
		{
			AllocateMemoryInt(h_Permutation_Matrix, PERMUTATION_MATRIX_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PERMUTATION_MATRIX");
			AllocateMemory(h_Permutation_Distribution, NULL_DISTRIBUTION_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PERMUTATION_DISTRIBUTION");             
			AllocateMemory(h_P_Values_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "PVALUES_MNI");
		}

		if (WRITE_RESIDUALS_EPI)
		{
			AllocateMemory(h_Residuals_EPI, RESIDUALS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "RESIDUALS_EPI");  
		}
	}
	else if (!REGRESS_ONLY && BETAS_ONLY && !PREPROCESSING_ONLY)
	{
		AllocateMemory(h_Beta_Volumes_MNI, BETA_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_MNI");
		AllocateMemory(h_Contrast_Volumes_MNI, STATISTICAL_MAPS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_MNI");    

	   	if (WRITE_ACTIVITY_EPI)
       	{
	    	AllocateMemory(h_Beta_Volumes_EPI, BETA_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_EPI");
	        AllocateMemory(h_Contrast_Volumes_EPI, STATISTICAL_MAPS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_EPI");
		}

		if (WRITE_ACTIVITY_T1)
    	{
	        AllocateMemory(h_Beta_Volumes_T1, BETA_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "BETA_VOLUMES_T1");
	        AllocateMemory(h_Contrast_Volumes_T1, STATISTICAL_MAPS_DATA_SIZE_T1, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_VOLUMES_T1");
		}
	}
	else if (PREPROCESSING_ONLY)
	{
		AllocateMemory(h_fMRI_Volumes_MNI, RESIDUALS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "fMRI_VOLUMES_MNI");             
	}
	else
	{
		AllocateMemory(h_Residuals_MNI, RESIDUALS_DATA_SIZE_MNI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "RESIDUALS_MNI");             

	    if (WRITE_ACTIVITY_EPI)
	    {
			AllocateMemory(h_Residuals_EPI, RESIDUALS_DATA_SIZE_EPI, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "RESIDUALS_EPI");             
		}
	}
    
	if (!PREPROCESSING_ONLY)
	{
	    AllocateMemory(h_X_GLM, GLM_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "DESIGN_MATRIX");
	    AllocateMemory(h_Highres_Regressors, HIGHRES_REGRESSORS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "HIGHRES_REGRESSOR");
	    AllocateMemory(h_LowpassFiltered_Regressors, HIGHRES_REGRESSORS_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "LOWPASSFILTERED_REGRESSOR");
	    AllocateMemory(h_xtxxt_GLM, GLM_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "DESIGN_MATRIX_INVERSE");
	    AllocateMemory(h_Contrasts, CONTRAST_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRASTS");
	    AllocateMemory(h_ctxtxc_GLM, CONTRAST_SCALAR_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "CONTRAST_SCALARS");
	    AllocateMemory(h_Design_Matrix, DESIGN_MATRIX_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "TOTAL_DESIGN_MATRIX");
    	AllocateMemory(h_Design_Matrix2, DESIGN_MATRIX_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "TOTAL_DESIGN_MATRIX2");
	}

	if (!PREPROCESSING_ONLY)
	{
	    AllocateMemory(h_AR1_Estimates_EPI, EPI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR1_ESTIMATES");
	    AllocateMemory(h_AR2_Estimates_EPI, EPI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR2_ESTIMATES");
	    AllocateMemory(h_AR3_Estimates_EPI, EPI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR3_ESTIMATES");
	    AllocateMemory(h_AR4_Estimates_EPI, EPI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR4_ESTIMATES");
	}

    if (WRITE_AR_ESTIMATES_MNI)
    {
        AllocateMemory(h_AR1_Estimates_MNI, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR1_ESTIMATES_MNI");
        AllocateMemory(h_AR2_Estimates_MNI, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR2_ESTIMATES_MNI");
        AllocateMemory(h_AR3_Estimates_MNI, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR3_ESTIMATES_MNI");
        AllocateMemory(h_AR4_Estimates_MNI, MNI_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR4_ESTIMATES_MNI");
    }

    if (WRITE_AR_ESTIMATES_T1)
    {
        AllocateMemory(h_AR1_Estimates_T1, T1_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR1_ESTIMATES_T1");
        AllocateMemory(h_AR2_Estimates_T1, T1_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR2_ESTIMATES_T1");
        AllocateMemory(h_AR3_Estimates_T1, T1_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR3_ESTIMATES_T1");
        AllocateMemory(h_AR4_Estimates_T1, T1_VOLUME_SIZE, allMemoryPointers, numberOfMemoryPointers, allNiftiImages, numberOfNiftiImages, allocatedHostMemory, "AR4_ESTIMATES_T1");
    }
    
	endTime = GetWallTime();
    
	if (VERBOS)
 	{
		printf("It took %f seconds to allocate memory\n",(float)(endTime - startTime));
	}
    
    // ------------------------------------------------
	// Read events for each regressor    	

	if (RAW_DESIGNMATRIX)
	{
		int designfile;
		if (!MULTIPLE_RUNS)
		{
			designfile = 4;
		}
		else
		{
			designfile = 5 + NUMBER_OF_RUNS;
		}

		int accumulatedTRs = 0;
		for (int run = 0; run < NUMBER_OF_RUNS; run++)
		{
		    // Open design file again
		    design.open(argv[designfile]);
		    // Read first two values again
		    design >> tempString; // NumRegressors as string
		    design >> NUMBER_OF_GLM_REGRESSORS;

			float tempFloat;	
			for (size_t t = 0; t < EPI_DATA_T_PER_RUN[run]; t++)
			{
				for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
				{
					if (! (design >> h_X_GLM[t + accumulatedTRs + r * EPI_DATA_T]) )
					{
						design.close();
				        printf("Could not read all values of the design file %s, aborting! Stopped reading at time point %zu for regressor %zu. Please check if the number of regressors and time points are correct. \n",argv[designfile],t,r);      
				        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
				        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
				        return EXIT_FAILURE;
					}
				}
			}	
			design.close();
			designfile++;
			accumulatedTRs += EPI_DATA_T_PER_RUN[run];
		}
	}
	else if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
		startTime = GetWallTime();

	    // Each line of the design file is a filename

		int designfile;
		if (!MULTIPLE_RUNS)
		{
			designfile = 4;
		}
		else
		{
			designfile = 5 + NUMBER_OF_RUNS;
		}

		// Reset highres regressors
	    for (size_t t = 0; t < NUMBER_OF_GLM_REGRESSORS * EPI_DATA_T * HIGHRES_FACTOR; t++)
    	{
			h_Highres_Regressors[t] = 0.0f;
		}

		int accumulatedTRs = 0;
		for (int run = 0; run < NUMBER_OF_RUNS; run++)
		{  
		    // Open design file again
		    design.open(argv[designfile]);
		    // Read first two values again
		    design >> tempString; // NumRegressors as string
		    design >> NUMBER_OF_GLM_REGRESSORS;

			if (!RAW_REGRESSORS)
			{    
			    // Loop over the number of regressors provided in the design file
			    for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
		    	{
			        // Each regressor is a filename, so try to open the file
			        std::ifstream regressor;
			        std::string filename;
			        design >> filename;
			        regressor.open(filename.c_str());
			        if (!regressor.good())
			        {
			            regressor.close();
			            printf("Unable to open the regressor file %s . Aborting! \n",filename.c_str());
			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
			            return EXIT_FAILURE;
			        }
        
			        // Read number of events for current regressor
			        regressor >> tempString; // NumEvents as string
			        std::string NE("NumEvents");
			        if (tempString.compare(NE) != 0)
			        {
    			        design.close();
			            printf("First element of each regressor file should be the string 'NumEvents', but it is %s for the regressor file %s. Aborting! \n",tempString.c_str(),filename.c_str());
			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
			            return EXIT_FAILURE;
    			    }
			        regressor >> NUMBER_OF_EVENTS;
	
					if (DEBUG)
					{
						printf("Number of events for regressor %zu is %i \n",r,NUMBER_OF_EVENTS);
					}
    	    
    			    // Loop over events
    			    for (int e = 0; e < NUMBER_OF_EVENTS; e++)
    			    {
    			        float onset;
    			        float duration;
    			        float value;
    	        
    			        // Read onset, duration and value for current event
						if (! (regressor >> onset) )
						{
							regressor.close();
    			            design.close();
    			            printf("Unable to read the onset for event %i in regressor file %s, aborting! Check the regressor file. \n",e,filename.c_str());
    			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			            return EXIT_FAILURE;
						}
	
    			        if (! (regressor >> duration) )
						{
							regressor.close();
    			            design.close();
    			            printf("Unable to read the duration for event %i in regressor file %s, aborting! Check the regressor file. \n",e,filename.c_str());
    			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			            return EXIT_FAILURE;
						}

						if (! (regressor >> value) )
						{
							regressor.close();
    			            design.close();
    			            printf("Unable to read the value for event %i in regressor file %s, aborting! Check the regressor file. \n",e,filename.c_str());
    			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			            return EXIT_FAILURE;
						}
    	    
						if (DEBUG)
						{
							printf("Event %i: Onset is %f, duration is %f and value is %f \n",e,onset,duration,value);
						}
    	        
    			        int start = (int)round(onset * (float)HIGHRES_FACTOR / TR);
    			        int activityLength = (int)round(duration * (float)HIGHRES_FACTOR / TR);
    	        
						if (DEBUG)
						{
							printf("Event %i: Start is %i, activity length is %i \n",e,start,activityLength);
						}

    			        // Put values into highres GLM
						bool warning = false;
    			        for (size_t i = 0; i < activityLength; i++)
    			        {
    			            if ((start + i) < (EPI_DATA_T_PER_RUN[run] * HIGHRES_FACTOR) )
    			            {
    			                h_Highres_Regressors[start + i + accumulatedTRs*HIGHRES_FACTOR + r * EPI_DATA_T*HIGHRES_FACTOR] = value;
    			            }
    			            else
    			            {
								warning = true;
 			                    //regressor.close();
    			                //design.close();
    			                //printf("For run %i, the activity start or duration for event %i in regressor file %s is longer than the duration of the fMRI data, aborting! Check the regressor file .\n",run+1,e,filename.c_str());	
    			                //FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			                //FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			                //return EXIT_FAILURE;
    			            }
    			        }           
						if (warning)
						{ 
				            printf("Warning:  For run %i, the activity start or duration for event %i in regressor file %s is longer than the duration of the fMRI data, ignoring the part after the experiment end. \n",run+1,e+1,filename.c_str());	
						}

    			    }
					regressor.close();	
				}
			}
			else if (RAW_REGRESSORS)
			{
				// Loop over the number of regressors provided in the design file
			    for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
    			{
			        // Each regressor is a filename, so try to open the file
			        std::ifstream regressor;
			        std::string filename;
			        design >> filename;
			        regressor.open(filename.c_str());
			        if (!regressor.good())
			        {
			            regressor.close();
			            printf("Unable to open the regressor file %s . Aborting! \n",filename.c_str());
			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
			            return EXIT_FAILURE;
			        }

					float value;
					int readValues = 0;
				    for (size_t t = 0; t < EPI_DATA_T_PER_RUN[run]; t++)
			    	{
						if (! (regressor >> value) )
						{
							regressor.close();
    			            design.close();
    			            printf("Unable to read the value for TR %zu in regressor file %s, aborting! Check the regressor file. \n",t,filename.c_str());
    			            FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			            FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			            return EXIT_FAILURE;
						}
						h_X_GLM[t + accumulatedTRs + r * EPI_DATA_T] = value;
						readValues++;
					}
		
					// Check if number of values is the same as the number of TRs
					if (readValues != EPI_DATA_T_PER_RUN[run])
					{
						regressor.close();
    			        design.close();
    			        printf("Number of values in regressor file %s is not the same as the number of TRs in the fMRI data (%i vs %zu), aborting! Check the regressor file. \n",filename.c_str(),readValues,EPI_DATA_T_PER_RUN[run]);
    			        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    			        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
    			        return EXIT_FAILURE;
					}
		
					regressor.close();
				}
			}
    		design.close();
			designfile++;
			accumulatedTRs += EPI_DATA_T_PER_RUN[run];
		}
	}

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY && !RAW_REGRESSORS && !RAW_DESIGNMATRIX)
	{
		// Lowpass filter highres regressors
		LowpassFilterRegressors(h_LowpassFiltered_Regressors,h_Highres_Regressors,EPI_DATA_T,HIGHRES_FACTOR,TR,NUMBER_OF_GLM_REGRESSORS);
        
	    // Downsample highres GLM and put values into regular GLM
	    for (size_t t = 0; t < EPI_DATA_T; t++)
	    {
			for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
		    {	
		        h_X_GLM[t + r * EPI_DATA_T] = h_LowpassFiltered_Regressors[t*HIGHRES_FACTOR + r * EPI_DATA_T * HIGHRES_FACTOR];
			}
	    }
	}

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
    	if (!BAYESIAN)
		{
			int contrastfile;
			if (!MULTIPLE_RUNS)
			{
				contrastfile = 5;
			}
			else
			{
				contrastfile = 5 + NUMBER_OF_RUNS*2;
			}

		    // Open contrast file again
		    contrasts.open(argv[contrastfile]);

		    // Read first two values again
			contrasts >> tempString; // NumRegressors as string
		    contrasts >> tempNumber;
		    contrasts >> tempString; // NumContrasts as string
		    contrasts >> tempNumber;
   
			// Read all contrast values
			for (size_t c = 0; c < NUMBER_OF_CONTRASTS; c++)
			{
				for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
				{
					if (! (contrasts >> h_Contrasts[r + c * NUMBER_OF_GLM_REGRESSORS]) )
					{
					    contrasts.close();
		                printf("Unable to read all the contrast values, aborting! Check the contrasts file. \n");
		                FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
		                FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
		                return EXIT_FAILURE;
					}
				}
			}
			contrasts.close();
		}

		endTime = GetWallTime();

		if (VERBOS)
	 	{
			printf("It took %f seconds to read regressors and contrasts\n",(float)(endTime - startTime));
		}
	}
    
	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
		// Write original design matrix to file
		if (WRITE_ORIGINAL_DESIGNMATRIX)
		{
			std::ofstream designmatrix;

			const char* extension = "_original_designmatrix.txt";
			char* filenameWithExtension;

			CreateFilename(filenameWithExtension, inputfMRI, extension, CHANGE_OUTPUT_FILENAME, outputFilename);
			
    		designmatrix.open(filenameWithExtension);    

			if ( designmatrix.good() )
			{
    			for (size_t t = 0; t < EPI_DATA_T; t++)
			    {
			    	for (size_t r = 0; r < NUMBER_OF_GLM_REGRESSORS; r++)
				    {
    		        	designmatrix << std::setprecision(6) << std::fixed << (double)h_X_GLM[t + r * EPI_DATA_T] << "  ";
					}
					designmatrix << std::endl;
				}
				designmatrix.close();
    		} 	
			else
			{
				designmatrix.close();
			    printf("Could not open the file for writing the original design matrix!\n");
			}
			free(filenameWithExtension);
		}
	}

    // ------------------------------------------------
	// Read data
    
	startTime = GetWallTime();
    
	// Convert fMRI data to floats
	size_t accumulatedTRs = 0;

	if (MULTIPLE_RUNS)
	{
		for (size_t run = 0; run < NUMBER_OF_RUNS; run++)
		{
			inputfMRI = allfMRINiftiImages[run];
	
		    if ( inputfMRI->datatype == DT_SIGNED_SHORT )
		    {
		        short int *p = (short int*)inputfMRI->data;
    			
		        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T_PER_RUN[run]; i++)
    		    {
    		        h_fMRI_Volumes[i + accumulatedTRs * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D] = (float)p[i];
    		    }
			}
		    else if ( inputfMRI->datatype == DT_UINT8 )
		    {
		        unsigned char *p = (unsigned char*)inputfMRI->data;
		    
		        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T_PER_RUN[run]; i++)
		        {
		            h_fMRI_Volumes[i + accumulatedTRs * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D] = (float)p[i];
		        }
		    }
		    else if ( inputfMRI->datatype == DT_UINT16 )
		    {
		        unsigned short int *p = (unsigned short int*)inputfMRI->data;
		    
		        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T_PER_RUN[run]; i++)
		        {
		            h_fMRI_Volumes[i + accumulatedTRs * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D] = (float)p[i];
		        }
		    }
		    else if ( inputfMRI->datatype == DT_FLOAT )
		    {		
		        float *p = (float*)inputfMRI->data;
		    
		        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T_PER_RUN[run]; i++)
		        {
		            h_fMRI_Volumes[i + accumulatedTRs * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D] = (float)p[i];
		        }
		    }
		    else
		    {
		        printf("Unknown data type in fMRI data for run %i, aborting!\n",run+1);
		        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
				FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
		        return EXIT_FAILURE;
		    }
			accumulatedTRs += EPI_DATA_T_PER_RUN[run];
		}
	}
	else
	{
	    if ( inputfMRI->datatype == DT_SIGNED_SHORT )
	    {
	        short int *p = (short int*)inputfMRI->data;
    		
	        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T; i++)
    	    {
    	        h_fMRI_Volumes[i] = (float)p[i];
    	    }
		}
	    else if ( inputfMRI->datatype == DT_UINT8 )
	    {
	        unsigned char *p = (unsigned char*)inputfMRI->data;
	    
	        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T; i++)
	        {
	            h_fMRI_Volumes[i] = (float)p[i];
	        }
	    }
	    else if ( inputfMRI->datatype == DT_UINT16 )
	    {
	        unsigned short int *p = (unsigned short int*)inputfMRI->data;
	    
	        for (size_t i = 0; i < EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T; i++)
	        {
	            h_fMRI_Volumes[i] = (float)p[i];
	        }
	    }
		// Correct data type, just copy the pointer
	    else if ( inputfMRI->datatype == DT_FLOAT )
	    {		
			h_fMRI_Volumes = (float*)inputfMRI->data;
	
			// Save the pointer in the pointer list
			allMemoryPointers[numberOfMemoryPointers] = (void*)h_fMRI_Volumes;
	        numberOfMemoryPointers++;
	    }	
	}

	if (MULTIPLE_RUNS)
	{
		// Free memory, as it is stored in a big array
		for (size_t run = 0; run < NUMBER_OF_RUNS; run++)
		{
			inputfMRI = allfMRINiftiImages[run];
			free(inputfMRI->data);
			inputfMRI->data = NULL;
		}
	}
	else
	{	
		// Free input fMRI data, it has been converted to floats
		if ( inputfMRI->datatype != DT_FLOAT )
		{		
			free(inputfMRI->data);
			inputfMRI->data = NULL;
		}
		// Pointer has been copied to h_fMRI_Volumes and pointer list, so set the input data pointer to NULL
		else
		{		
			inputfMRI->data = NULL;
		}
	}



	// Convert T1 volume to floats
    if ( inputT1->datatype == DT_SIGNED_SHORT )
    {
        short int *p = (short int*)inputT1->data;
    
        for (size_t i = 0; i < T1_DATA_W * T1_DATA_H * T1_DATA_D; i++)
        {
            h_T1_Volume[i] = (float)p[i];
        }
    }
    else if ( inputT1->datatype == DT_UINT8 )
    {
        unsigned char *p = (unsigned char*)inputT1->data;
    
        for (size_t i = 0; i < T1_DATA_W * T1_DATA_H * T1_DATA_D; i++)
        {
            h_T1_Volume[i] = (float)p[i];
        }
    }
    else if ( inputT1->datatype == DT_FLOAT )
    {
        float *p = (float*)inputT1->data;
    
        for (size_t i = 0; i < T1_DATA_W * T1_DATA_H * T1_DATA_D; i++)
        {
            h_T1_Volume[i] = p[i];
        }
    }
    else
    {
        printf("Unknown data type in T1 volume, aborting!\n");
        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
		FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }

	// Convert MNI volume to floats
    if ( inputMNI->datatype == DT_SIGNED_SHORT )
    {
        short int *p = (short int*)inputMNI->data;
    
        for (size_t i = 0; i < MNI_DATA_W * MNI_DATA_H * MNI_DATA_D; i++)
        {
            h_MNI_Brain_Volume[i] = (float)p[i];
        }
    }
    else if ( inputMNI->datatype == DT_UINT8 )
    {
        unsigned char *p = (unsigned char*)inputMNI->data;
    
        for (size_t i = 0; i < MNI_DATA_W * MNI_DATA_H * MNI_DATA_D; i++)
        {
            h_MNI_Brain_Volume[i] = (float)p[i];
        }
    }
    else if ( inputMNI->datatype == DT_FLOAT )
    {
        float *p = (float*)inputMNI->data;
    
        for (size_t i = 0; i < MNI_DATA_W * MNI_DATA_H * MNI_DATA_D; i++)
        {
            h_MNI_Brain_Volume[i] = p[i];
        }
    }
    else
    {
        printf("Unknown data type in MNI volume, aborting!\n");
        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
		FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;
    }

	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to convert data to floats\n",(float)(endTime - startTime));
	}

	startTime = GetWallTime();

	std::string filter1RealLinearPathAndName;
	std::string filter1ImagLinearPathAndName;
	std::string filter2RealLinearPathAndName;
	std::string filter2ImagLinearPathAndName;
	std::string filter3RealLinearPathAndName;
	std::string filter3ImagLinearPathAndName;

	filter1RealLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter1ImagLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter2RealLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter2ImagLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter3RealLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter3ImagLinearPathAndName.append(getenv("BROCCOLI_DIR"));

	filter1RealLinearPathAndName.append("filters/filter1_real_linear_registration.bin");
	filter1ImagLinearPathAndName.append("filters/filter1_imag_linear_registration.bin");
	filter2RealLinearPathAndName.append("filters/filter2_real_linear_registration.bin");
	filter2ImagLinearPathAndName.append("filters/filter2_imag_linear_registration.bin");
	filter3RealLinearPathAndName.append("filters/filter3_real_linear_registration.bin");
	filter3ImagLinearPathAndName.append("filters/filter3_imag_linear_registration.bin");
    
    // Read quadrature filters for linear registration, three real valued and three imaginary valued
	ReadBinaryFile(h_Quadrature_Filter_1_Linear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter1RealLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_1_Linear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter1ImagLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_2_Linear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter2RealLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_2_Linear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter2ImagLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_3_Linear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter3RealLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_3_Linear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter3ImagLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 

	std::string filter1RealNonLinearPathAndName;
	std::string filter1ImagNonLinearPathAndName;
	std::string filter2RealNonLinearPathAndName;
	std::string filter2ImagNonLinearPathAndName;
	std::string filter3RealNonLinearPathAndName;
	std::string filter3ImagNonLinearPathAndName;
	std::string filter4RealNonLinearPathAndName;
	std::string filter4ImagNonLinearPathAndName;
	std::string filter5RealNonLinearPathAndName;
	std::string filter5ImagNonLinearPathAndName;
	std::string filter6RealNonLinearPathAndName;
	std::string filter6ImagNonLinearPathAndName;

	filter1RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter1ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter2RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter2ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter3RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter3ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter4RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter4ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter5RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter5ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter6RealNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));
	filter6ImagNonLinearPathAndName.append(getenv("BROCCOLI_DIR"));

	filter1RealNonLinearPathAndName.append("filters/filter1_real_nonlinear_registration.bin");
	filter1ImagNonLinearPathAndName.append("filters/filter1_imag_nonlinear_registration.bin");
	filter2RealNonLinearPathAndName.append("filters/filter2_real_nonlinear_registration.bin");
	filter2ImagNonLinearPathAndName.append("filters/filter2_imag_nonlinear_registration.bin");
	filter3RealNonLinearPathAndName.append("filters/filter3_real_nonlinear_registration.bin");
	filter3ImagNonLinearPathAndName.append("filters/filter3_imag_nonlinear_registration.bin");
	filter4RealNonLinearPathAndName.append("filters/filter4_real_nonlinear_registration.bin");
	filter4ImagNonLinearPathAndName.append("filters/filter4_imag_nonlinear_registration.bin");
	filter5RealNonLinearPathAndName.append("filters/filter5_real_nonlinear_registration.bin");
	filter5ImagNonLinearPathAndName.append("filters/filter5_imag_nonlinear_registration.bin");
	filter6RealNonLinearPathAndName.append("filters/filter6_real_nonlinear_registration.bin");
	filter6ImagNonLinearPathAndName.append("filters/filter6_imag_nonlinear_registration.bin");

	// Read quadrature filters for nonLinear registration, six real valued and six imaginary valued
	ReadBinaryFile(h_Quadrature_Filter_1_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter1RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_1_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter1ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_2_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter2RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_2_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter2ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_3_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter3RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_3_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter3ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_4_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter4RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_4_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter4ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_5_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter5RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_5_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter5ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_6_NonLinear_Registration_Real,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter6RealNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
	ReadBinaryFile(h_Quadrature_Filter_6_NonLinear_Registration_Imag,IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE*IMAGE_REGISTRATION_FILTER_SIZE,filter6ImagNonLinearPathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 

	std::string projectionTensor1PathAndName;
	std::string projectionTensor2PathAndName;
	std::string projectionTensor3PathAndName;
	std::string projectionTensor4PathAndName;
	std::string projectionTensor5PathAndName;
	std::string projectionTensor6PathAndName;

	projectionTensor1PathAndName.append(getenv("BROCCOLI_DIR"));
	projectionTensor2PathAndName.append(getenv("BROCCOLI_DIR"));
	projectionTensor3PathAndName.append(getenv("BROCCOLI_DIR"));
	projectionTensor4PathAndName.append(getenv("BROCCOLI_DIR"));
	projectionTensor5PathAndName.append(getenv("BROCCOLI_DIR"));
	projectionTensor6PathAndName.append(getenv("BROCCOLI_DIR"));

	projectionTensor1PathAndName.append("filters/projection_tensor1.bin");
	projectionTensor2PathAndName.append("filters/projection_tensor2.bin");
	projectionTensor3PathAndName.append("filters/projection_tensor3.bin");
	projectionTensor4PathAndName.append("filters/projection_tensor4.bin");
	projectionTensor5PathAndName.append("filters/projection_tensor5.bin");
	projectionTensor6PathAndName.append("filters/projection_tensor6.bin");

    // Read projection tensors   
    ReadBinaryFile(h_Projection_Tensor_1,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor1PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Projection_Tensor_2,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor2PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Projection_Tensor_3,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor3PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Projection_Tensor_4,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor4PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Projection_Tensor_5,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor5PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Projection_Tensor_6,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,projectionTensor6PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
        
	std::string filterDirections1PathAndName;
	std::string filterDirections2PathAndName;
	std::string filterDirections3PathAndName;

	filterDirections1PathAndName.append(getenv("BROCCOLI_DIR"));
	filterDirections2PathAndName.append(getenv("BROCCOLI_DIR"));
	filterDirections3PathAndName.append(getenv("BROCCOLI_DIR"));

	filterDirections1PathAndName.append("filters/filter_directions_x.bin");
	filterDirections2PathAndName.append("filters/filter_directions_y.bin");
	filterDirections3PathAndName.append("filters/filter_directions_z.bin");

    // Read filter directions
    ReadBinaryFile(h_Filter_Directions_X,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,filterDirections1PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages);
    ReadBinaryFile(h_Filter_Directions_Y,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,filterDirections2PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages); 
    ReadBinaryFile(h_Filter_Directions_Z,NUMBER_OF_FILTERS_FOR_NONLINEAR_REGISTRATION,filterDirections3PathAndName.c_str(),allMemoryPointers,numberOfMemoryPointers,allNiftiImages,numberOfNiftiImages);  

	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to read all binary files\n",(float)(endTime - startTime));
	}       
    
    //------------------------
    
	startTime = GetWallTime();

	// Initialize BROCCOLI
    BROCCOLI_LIB BROCCOLI(OPENCL_PLATFORM,OPENCL_DEVICE,2,VERBOS); // 2 = Bash wrapper

	endTime = GetWallTime();

	if (VERBOS)
 	{
		printf("It took %f seconds to initiate BROCCOLI\n",(float)(endTime - startTime));
	}

    // Print build info to file (always)
	std::vector<std::string> buildInfo = BROCCOLI.GetOpenCLBuildInfo();
	std::vector<std::string> kernelFileNames = BROCCOLI.GetKernelFileNames();

	std::string buildInfoPath;
	buildInfoPath.append(getenv("BROCCOLI_DIR"));
	buildInfoPath.append("compiled/Kernels/");

	for (int k = 0; k < BROCCOLI.GetNumberOfKernelFiles(); k++)
	{
		std::string temp = buildInfoPath;
		temp.append("buildInfo_");
		temp.append(BROCCOLI.GetOpenCLPlatformName());
		temp.append("_");	
		temp.append(BROCCOLI.GetOpenCLDeviceName());
		temp.append("_");	
		std::string name = kernelFileNames[k];
		// Remove "kernel" and ".cpp" from kernel filename
		name = name.substr(0,name.size()-4);
		name = name.substr(6,name.size());
		temp.append(name);
		temp.append(".txt");
		fp = fopen(temp.c_str(),"w");
		if (fp == NULL)
		{     
		    printf("Could not open %s for writing ! \n",temp.c_str());
		}
		else
		{	
			if (buildInfo[k].c_str() != NULL)
			{
			    int error = fputs(buildInfo[k].c_str(),fp);
			    if (error == EOF)
			    {
			        printf("Could not write to %s ! \n",temp.c_str());
			    }
			}
			fclose(fp);
		}
	}

    // Something went wrong...
    if ( !BROCCOLI.GetOpenCLInitiated() )
    {  
        printf("Initialization error is \"%s\" \n",BROCCOLI.GetOpenCLInitializationError().c_str());
		printf("OpenCL error is \"%s\" \n",BROCCOLI.GetOpenCLError());
		
        // Print create kernel errors
        int* createKernelErrors = BROCCOLI.GetOpenCLCreateKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createKernelErrors[i] != 0)
            {
                printf("Create kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(createKernelErrors[i]));
            }
        } 
       
        printf("OpenCL initialization failed, aborting! \nSee buildInfo* for output of OpenCL compilation!\n");      
        FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
        FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);
        return EXIT_FAILURE;  
    }
	// Initialization went OK
    else
    {
		BROCCOLI.SetAllocatedHostMemory(allocatedHostMemory);

        BROCCOLI.SetEPIWidth(EPI_DATA_W);
        BROCCOLI.SetEPIHeight(EPI_DATA_H);
        BROCCOLI.SetEPIDepth(EPI_DATA_D);
        BROCCOLI.SetEPITimepoints(EPI_DATA_T);     
		BROCCOLI.SetEPITimepointsPerRun(EPI_DATA_T_PER_RUN);
        BROCCOLI.SetEPITR(TR);  
		BROCCOLI.SetNumberOfRuns(NUMBER_OF_RUNS);
        BROCCOLI.SetEPISliceOrder(SLICE_ORDER); 
		BROCCOLI.SetCustomSliceTimes(h_Custom_Slice_Times);
		BROCCOLI.SetCustomReferenceSlice(SLICE_CUSTOM_REF);

		BROCCOLI.SetApplySliceTimingCorrection(APPLY_SLICE_TIMING_CORRECTION);
		BROCCOLI.SetApplyMotionCorrection(APPLY_MOTION_CORRECTION);
		BROCCOLI.SetApplySmoothing(APPLY_SMOOTHING);

        BROCCOLI.SetT1Width(T1_DATA_W);
        BROCCOLI.SetT1Height(T1_DATA_H);
        BROCCOLI.SetT1Depth(T1_DATA_D);

        BROCCOLI.SetMNIWidth(MNI_DATA_W);
        BROCCOLI.SetMNIHeight(MNI_DATA_H);
        BROCCOLI.SetMNIDepth(MNI_DATA_D);
        
        BROCCOLI.SetInputfMRIVolumes(h_fMRI_Volumes);
        BROCCOLI.SetInputT1Volume(h_T1_Volume);
        //BROCCOLI.SetInputMNIVolume(h_MNI_Volume);
        BROCCOLI.SetInputMNIBrainVolume(h_MNI_Brain_Volume);
		//BROCCOLI.SetInputMNIBrainMask(h_MNI_Brain_Mask);
        
        BROCCOLI.SetEPIVoxelSizeX(EPI_VOXEL_SIZE_X);
        BROCCOLI.SetEPIVoxelSizeY(EPI_VOXEL_SIZE_Y);
        BROCCOLI.SetEPIVoxelSizeZ(EPI_VOXEL_SIZE_Z);       
        BROCCOLI.SetT1VoxelSizeX(T1_VOXEL_SIZE_X);
        BROCCOLI.SetT1VoxelSizeY(T1_VOXEL_SIZE_Y);
        BROCCOLI.SetT1VoxelSizeZ(T1_VOXEL_SIZE_Z);   
        BROCCOLI.SetMNIVoxelSizeX(MNI_VOXEL_SIZE_X);
        BROCCOLI.SetMNIVoxelSizeY(MNI_VOXEL_SIZE_Y);
        BROCCOLI.SetMNIVoxelSizeZ(MNI_VOXEL_SIZE_Z); 
        BROCCOLI.SetInterpolationMode(LINEAR);
    
        BROCCOLI.SetNumberOfIterationsForLinearImageRegistration(NUMBER_OF_ITERATIONS_FOR_LINEAR_IMAGE_REGISTRATION);
        BROCCOLI.SetNumberOfIterationsForNonLinearImageRegistration(NUMBER_OF_ITERATIONS_FOR_NONLINEAR_IMAGE_REGISTRATION);
        BROCCOLI.SetImageRegistrationFilterSize(IMAGE_REGISTRATION_FILTER_SIZE);    
        BROCCOLI.SetLinearImageRegistrationFilters(h_Quadrature_Filter_1_Linear_Registration_Real, h_Quadrature_Filter_1_Linear_Registration_Imag, h_Quadrature_Filter_2_Linear_Registration_Real, h_Quadrature_Filter_2_Linear_Registration_Imag, h_Quadrature_Filter_3_Linear_Registration_Real, h_Quadrature_Filter_3_Linear_Registration_Imag);
        BROCCOLI.SetNonLinearImageRegistrationFilters(h_Quadrature_Filter_1_NonLinear_Registration_Real, h_Quadrature_Filter_1_NonLinear_Registration_Imag, h_Quadrature_Filter_2_NonLinear_Registration_Real, h_Quadrature_Filter_2_NonLinear_Registration_Imag, h_Quadrature_Filter_3_NonLinear_Registration_Real, h_Quadrature_Filter_3_NonLinear_Registration_Imag, h_Quadrature_Filter_4_NonLinear_Registration_Real, h_Quadrature_Filter_4_NonLinear_Registration_Imag, h_Quadrature_Filter_5_NonLinear_Registration_Real, h_Quadrature_Filter_5_NonLinear_Registration_Imag, h_Quadrature_Filter_6_NonLinear_Registration_Real, h_Quadrature_Filter_6_NonLinear_Registration_Imag);    
        BROCCOLI.SetProjectionTensorMatrixFirstFilter(h_Projection_Tensor_1[0],h_Projection_Tensor_1[1],h_Projection_Tensor_1[2],h_Projection_Tensor_1[3],h_Projection_Tensor_1[4],h_Projection_Tensor_1[5]);
        BROCCOLI.SetProjectionTensorMatrixSecondFilter(h_Projection_Tensor_2[0],h_Projection_Tensor_2[1],h_Projection_Tensor_2[2],h_Projection_Tensor_2[3],h_Projection_Tensor_2[4],h_Projection_Tensor_2[5]);
        BROCCOLI.SetProjectionTensorMatrixThirdFilter(h_Projection_Tensor_3[0],h_Projection_Tensor_3[1],h_Projection_Tensor_3[2],h_Projection_Tensor_3[3],h_Projection_Tensor_3[4],h_Projection_Tensor_3[5]);
        BROCCOLI.SetProjectionTensorMatrixFourthFilter(h_Projection_Tensor_4[0],h_Projection_Tensor_4[1],h_Projection_Tensor_4[2],h_Projection_Tensor_4[3],h_Projection_Tensor_4[4],h_Projection_Tensor_4[5]);
        BROCCOLI.SetProjectionTensorMatrixFifthFilter(h_Projection_Tensor_5[0],h_Projection_Tensor_5[1],h_Projection_Tensor_5[2],h_Projection_Tensor_5[3],h_Projection_Tensor_5[4],h_Projection_Tensor_5[5]);
        BROCCOLI.SetProjectionTensorMatrixSixthFilter(h_Projection_Tensor_6[0],h_Projection_Tensor_6[1],h_Projection_Tensor_6[2],h_Projection_Tensor_6[3],h_Projection_Tensor_6[4],h_Projection_Tensor_6[5]);
        BROCCOLI.SetFilterDirections(h_Filter_Directions_X, h_Filter_Directions_Y, h_Filter_Directions_Z);
    
        BROCCOLI.SetNumberOfIterationsForMotionCorrection(NUMBER_OF_ITERATIONS_FOR_MOTION_CORRECTION);    
        BROCCOLI.SetCoarsestScaleT1MNI(COARSEST_SCALE_T1_MNI);
        BROCCOLI.SetCoarsestScaleEPIT1(COARSEST_SCALE_EPI_T1);
        BROCCOLI.SetMMT1ZCUT(MM_T1_Z_CUT);   
        BROCCOLI.SetMMEPIZCUT(MM_EPI_Z_CUT);   
        BROCCOLI.SetEPISmoothingAmount(EPI_SMOOTHING_AMOUNT);
        BROCCOLI.SetARSmoothingAmount(AR_SMOOTHING_AMOUNT);

		BROCCOLI.SetSaveInterpolatedT1(WRITE_INTERPOLATED_T1);
		BROCCOLI.SetSaveAlignedT1MNILinear(WRITE_ALIGNED_T1_MNI_LINEAR);
		BROCCOLI.SetSaveAlignedT1MNINonLinear(WRITE_ALIGNED_T1_MNI_NONLINEAR);	
		BROCCOLI.SetSaveAlignedEPIT1(WRITE_ALIGNED_EPI_T1);	
		BROCCOLI.SetSaveAlignedEPIMNI(WRITE_ALIGNED_EPI_MNI);	

		BROCCOLI.SetSaveEPIMask(WRITE_EPI_MASK);
		BROCCOLI.SetSaveMNIMask(WRITE_MNI_MASK);
		BROCCOLI.SetSaveSliceTimingCorrected(WRITE_SLICETIMING_CORRECTED);
		BROCCOLI.SetSaveMotionCorrected(WRITE_MOTION_CORRECTED);
		BROCCOLI.SetSaveSmoothed(WRITE_SMOOTHED);				

		BROCCOLI.SetSaveActivityEPI(WRITE_ACTIVITY_EPI);
		BROCCOLI.SetSaveActivityT1(WRITE_ACTIVITY_T1);
		BROCCOLI.SetSaveDesignMatrix(WRITE_DESIGNMATRIX);
		BROCCOLI.SetSaveAREstimatesEPI(WRITE_AR_ESTIMATES_EPI);
		BROCCOLI.SetSaveAREstimatesT1(WRITE_AR_ESTIMATES_T1);
		BROCCOLI.SetSaveAREstimatesMNI(WRITE_AR_ESTIMATES_MNI);
		BROCCOLI.SetSaveUnwhitenedResults(WRITE_UNWHITENED_RESULTS);
		BROCCOLI.SetSaveResidualsEPI(WRITE_RESIDUALS_EPI);

        BROCCOLI.SetOutputSliceTimingCorrectedfMRIVolumes(h_Slice_Timing_Corrected_fMRI_Volumes);
        BROCCOLI.SetOutputMotionCorrectedfMRIVolumes(h_Motion_Corrected_fMRI_Volumes);
        BROCCOLI.SetOutputSmoothedfMRIVolumes(h_Smoothed_fMRI_Volumes);
    
		BROCCOLI.SetPermuteFirstLevel(PERMUTE);
        BROCCOLI.SetNumberOfPermutations(NUMBER_OF_PERMUTATIONS);
		BROCCOLI.SetPermutationMatrix(h_Permutation_Matrix);      
        BROCCOLI.SetOutputPermutationDistribution(h_Permutation_Distribution);

        BROCCOLI.SetRawRegressors(RAW_REGRESSORS);
        BROCCOLI.SetRawDesignMatrix(RAW_DESIGNMATRIX);
        BROCCOLI.SetRegressMotion(REGRESS_MOTION);
        BROCCOLI.SetRegressGlobalMean(REGRESS_GLOBALMEAN);
        BROCCOLI.SetTemporalDerivatives(USE_TEMPORAL_DERIVATIVES);
        //BROCCOLI.SetRegressConfounds(REGRESS_CONFOUNDS);

        BROCCOLI.SetNumberOfMCMCIterations(NUMBER_OF_MCMC_ITERATIONS);
    
        if (REGRESS_CONFOUNDS == 1)
        {
            //BROCCOLI.SetNumberOfConfoundRegressors(NUMBER_OF_CONFOUND_REGRESSORS);
            //BROCCOLI.SetConfoundRegressors(h_X_GLM_Confounds);
        }
    
        BROCCOLI.SetNumberOfGLMRegressors(NUMBER_OF_GLM_REGRESSORS);
		BROCCOLI.SetNumberOfDetrendingRegressors(NUMBER_OF_DETRENDING_REGRESSORS);
        BROCCOLI.SetNumberOfContrasts(NUMBER_OF_CONTRASTS);    
        BROCCOLI.SetDesignMatrix(h_X_GLM, h_xtxxt_GLM);
        BROCCOLI.SetContrasts(h_Contrasts);
        BROCCOLI.SetGLMScalars(h_ctxtxc_GLM);

        BROCCOLI.SetInferenceMode(INFERENCE_MODE);        
        BROCCOLI.SetClusterDefiningThreshold(CLUSTER_DEFINING_THRESHOLD);
        BROCCOLI.SetSignificanceLevel(SIGNIFICANCE_LEVEL);		
    
        BROCCOLI.SetOutputT1MNIRegistrationParameters(h_T1_MNI_Registration_Parameters);
        BROCCOLI.SetOutputEPIT1RegistrationParameters(h_EPI_T1_Registration_Parameters);
        BROCCOLI.SetOutputEPIMNIRegistrationParameters(h_EPI_MNI_Registration_Parameters);
        BROCCOLI.SetOutputMotionParameters(h_Motion_Parameters);

        BROCCOLI.SetOutputInterpolatedT1Volume(h_Interpolated_T1_Volume);
        BROCCOLI.SetOutputAlignedT1VolumeLinear(h_Aligned_T1_Volume_Linear);
        BROCCOLI.SetOutputAlignedT1VolumeNonLinear(h_Aligned_T1_Volume_NonLinear);
        BROCCOLI.SetOutputAlignedEPIVolumeT1(h_Aligned_EPI_Volume_T1);
        BROCCOLI.SetOutputAlignedEPIVolumeMNILinear(h_Aligned_EPI_Volume_MNI_Linear);
        BROCCOLI.SetOutputAlignedEPIVolumeMNINonlinear(h_Aligned_EPI_Volume_MNI_Nonlinear);
        BROCCOLI.SetOutputEPIMask(h_EPI_Mask);
        BROCCOLI.SetOutputMNIMask(h_MNI_Mask);
        BROCCOLI.SetOutputSliceTimingCorrectedfMRIVolumes(h_Slice_Timing_Corrected_fMRI_Volumes);
        BROCCOLI.SetOutputMotionCorrectedfMRIVolumes(h_Motion_Corrected_fMRI_Volumes);
        BROCCOLI.SetOutputSmoothedfMRIVolumes(h_Smoothed_fMRI_Volumes);

        BROCCOLI.SetOutputBetaVolumesEPI(h_Beta_Volumes_EPI);
        BROCCOLI.SetOutputContrastVolumesEPI(h_Contrast_Volumes_EPI);
        BROCCOLI.SetOutputStatisticalMapsEPI(h_Statistical_Maps_EPI);
        BROCCOLI.SetOutputPValuesEPI(h_P_Values_EPI);

        BROCCOLI.SetOutputBetaVolumesNoWhiteningEPI(h_Beta_Volumes_No_Whitening_EPI);
        BROCCOLI.SetOutputContrastVolumesNoWhiteningEPI(h_Contrast_Volumes_No_Whitening_EPI);
        BROCCOLI.SetOutputStatisticalMapsNoWhiteningEPI(h_Statistical_Maps_No_Whitening_EPI);

        BROCCOLI.SetOutputBetaVolumesT1(h_Beta_Volumes_T1);
        BROCCOLI.SetOutputContrastVolumesT1(h_Contrast_Volumes_T1);
        BROCCOLI.SetOutputStatisticalMapsT1(h_Statistical_Maps_T1);
        BROCCOLI.SetOutputPValuesT1(h_P_Values_T1);

        BROCCOLI.SetOutputBetaVolumesNoWhiteningT1(h_Beta_Volumes_No_Whitening_T1);
        BROCCOLI.SetOutputContrastVolumesNoWhiteningT1(h_Contrast_Volumes_No_Whitening_T1);
        BROCCOLI.SetOutputStatisticalMapsNoWhiteningT1(h_Statistical_Maps_No_Whitening_T1);

        BROCCOLI.SetOutputBetaVolumesMNI(h_Beta_Volumes_MNI);
        BROCCOLI.SetOutputContrastVolumesMNI(h_Contrast_Volumes_MNI);
        BROCCOLI.SetOutputStatisticalMapsMNI(h_Statistical_Maps_MNI);
        BROCCOLI.SetOutputPValuesMNI(h_P_Values_MNI);

        BROCCOLI.SetOutputBetaVolumesNoWhiteningMNI(h_Beta_Volumes_No_Whitening_MNI);
        BROCCOLI.SetOutputContrastVolumesNoWhiteningMNI(h_Contrast_Volumes_No_Whitening_MNI);
        BROCCOLI.SetOutputStatisticalMapsNoWhiteningMNI(h_Statistical_Maps_No_Whitening_MNI);

		BROCCOLI.SetBayesian(BAYESIAN);
		BROCCOLI.SetPreprocessingOnly(PREPROCESSING_ONLY);
		BROCCOLI.SetRegressOnly(REGRESS_ONLY);
		BROCCOLI.SetBetasOnly(BETAS_ONLY);
        BROCCOLI.SetOutputResidualsMNI(h_Residuals_MNI);
		BROCCOLI.SetOutputfMRIVolumesMNI(h_fMRI_Volumes_MNI);
        BROCCOLI.SetOutputResidualsEPI(h_Residuals_EPI);

        //BROCCOLI.SetOutputResidualVariances(h_Residual_Variances);
        BROCCOLI.SetOutputAREstimatesEPI(h_AR1_Estimates_EPI, h_AR2_Estimates_EPI, h_AR3_Estimates_EPI, h_AR4_Estimates_EPI);
        BROCCOLI.SetOutputAREstimatesMNI(h_AR1_Estimates_MNI, h_AR2_Estimates_MNI, h_AR3_Estimates_MNI, h_AR4_Estimates_MNI);
        BROCCOLI.SetOutputAREstimatesT1(h_AR1_Estimates_T1, h_AR2_Estimates_T1, h_AR3_Estimates_T1, h_AR4_Estimates_T1);
        //BROCCOLI.SetOutputWhitenedModels(h_Whitened_Models);
		    
		BROCCOLI.SetPrint(PRINT);

        BROCCOLI.SetOutputDesignMatrix(h_Design_Matrix, h_Design_Matrix2);
        
		startTime = GetWallTime();
       	BROCCOLI.PerformFirstLevelAnalysisWrapper();	        
		endTime = GetWallTime();

		if (VERBOS)
	 	{
			printf("\nIt took %f seconds to run the first level analysis\n",(float)(endTime - startTime));
		}

        // Print create buffer errors
        int* createBufferErrors = BROCCOLI.GetOpenCLCreateBufferErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createBufferErrors[i] != 0)
            {
                printf("Create buffer error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(createBufferErrors[i]));
            }
        }
        
        // Print create kernel errors
        int* createKernelErrors = BROCCOLI.GetOpenCLCreateKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (createKernelErrors[i] != 0)
            {
                printf("Create kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(createKernelErrors[i]));
            }
        } 

        // Print run kernel errors
        int* runKernelErrors = BROCCOLI.GetOpenCLRunKernelErrors();
        for (int i = 0; i < BROCCOLI.GetNumberOfOpenCLKernels(); i++)
        {
            if (runKernelErrors[i] != 0)
            {
                printf("Run kernel error for kernel '%s' is '%s' \n",BROCCOLI.GetOpenCLKernelName(i),BROCCOLI.GetOpenCLErrorMessage(runKernelErrors[i]));
            }
        } 
    }
    
    startTime = GetWallTime();

	if (WRITE_TRANSFORMATION_MATRICES)
	{		
		const char* extension1 = "_affinematrix_t1_mni.txt";
		char* filenameWithExtension;

		CreateFilename(filenameWithExtension, inputT1, extension1, CHANGE_OUTPUT_FILENAME, outputFilename);

		std::ofstream matrix;
	    matrix.open(filenameWithExtension);      
	    if ( matrix.good() )
	    {
	        matrix.precision(6);

	        matrix << h_T1_MNI_Registration_Parameters[3]+1.0f << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[4] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[5] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[0] << std::endl;
	        matrix << h_T1_MNI_Registration_Parameters[6] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[7] + 1.0f << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[8] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[1] << std::endl;
	        matrix << h_T1_MNI_Registration_Parameters[9] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[10] << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[11] + 1.0f << std::setw(2) << " " << h_T1_MNI_Registration_Parameters[2] << std::endl;
	        matrix << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 1.0f << std::endl;

	        matrix.close();
	    }
	    else
	    {
	        printf("Could not open %s for writing!\n",filenameWithExtension);
	    }
		free(filenameWithExtension);



		const char* extension2 = "_affinematrix_epi_t1.txt";
		CreateFilename(filenameWithExtension, inputfMRI, extension2, CHANGE_OUTPUT_FILENAME, outputFilename);

	    matrix.open(filenameWithExtension);      
	    if ( matrix.good() )
	    {
	        matrix.precision(6);

	        matrix << h_EPI_T1_Registration_Parameters[3]+1.0f << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[4] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[5] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[0] << std::endl;
	        matrix << h_EPI_T1_Registration_Parameters[6] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[7] + 1.0f << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[8] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[1] << std::endl;
	        matrix << h_EPI_T1_Registration_Parameters[9] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[10] << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[11] + 1.0f << std::setw(2) << " " << h_EPI_T1_Registration_Parameters[2] << std::endl;
	        matrix << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 1.0f << std::endl;

	        matrix.close();
	    }
	    else
	    {
	        printf("Could not open %s for writing!\n",filenameWithExtension);
	    }
		free(filenameWithExtension);




		const char* extension3 = "_affinematrix_epi_mni.txt";
		CreateFilename(filenameWithExtension, inputfMRI, extension3, CHANGE_OUTPUT_FILENAME, outputFilename);

	    matrix.open(filenameWithExtension);      
	    if ( matrix.good() )
	    {
	        matrix.precision(6);

	        matrix << h_EPI_MNI_Registration_Parameters[3]+1.0f << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[4] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[5] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[0] << std::endl;
	        matrix << h_EPI_MNI_Registration_Parameters[6] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[7] + 1.0f << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[8] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[1] << std::endl;
	        matrix << h_EPI_MNI_Registration_Parameters[9] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[10] << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[11] + 1.0f << std::setw(2) << " " << h_EPI_MNI_Registration_Parameters[2] << std::endl;
	        matrix << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 0.0f << std::setw(2) << " " << 1.0f << std::endl;

	        matrix.close();
	    }
	    else
	    {
	        printf("Could not open %s for writing!\n",filenameWithExtension);
	    }
		free(filenameWithExtension);

	}

	inputfMRI = allfMRINiftiImages[0];

	// Write total design matrix to file
	if (WRITE_DESIGNMATRIX)
	{
		std::ofstream designmatrix;

		const char* extension = "_total_designmatrix.txt";
		char* filenameWithExtension;

		CreateFilename(filenameWithExtension, inputfMRI, extension, CHANGE_OUTPUT_FILENAME, outputFilename);

    	designmatrix.open(filenameWithExtension);    

	    if ( designmatrix.good() )
	    {
    	    for (size_t t = 0; t < EPI_DATA_T; t++)
	        {
	    	    for (size_t r = 0; r < NUMBER_OF_TOTAL_GLM_REGRESSORS; r++)
		        {
            		designmatrix << std::setprecision(6) << std::fixed << (double)h_Design_Matrix[t + r * EPI_DATA_T] << "  ";
				}
				designmatrix << std::endl;
			}
		    designmatrix.close();
        } 	
	    else
	    {
			designmatrix.close();
	        printf("Could not open the file for writing the total design matrix!\n");
	    }
		free(filenameWithExtension);
	}


	if (WRITE_MOTION_PARAMETERS)
	{
		// Print motion parameters to file
	    std::ofstream motion;

	  	const char* extension = "_motionparameters.1D";
		char* filenameWithExtension;

		CreateFilename(filenameWithExtension, inputfMRI, extension, CHANGE_OUTPUT_FILENAME, outputFilename);

    	// Add the extension
    	strcat(filenameWithExtension,extension);

    	motion.open(filenameWithExtension);      

    	if ( motion.good() )
    	{		  	
	        motion.precision(6);
    	    for (size_t t = 0; t < EPI_DATA_T; t++)
    	    {    
	            motion << h_Motion_Parameters[t + 4*EPI_DATA_T] << std::setw(2) << " " << -h_Motion_Parameters[t + 3*EPI_DATA_T] << std::setw(2) << " " << h_Motion_Parameters[t + 5*EPI_DATA_T] << std::setw(2) << " " << -h_Motion_Parameters[t + 2*EPI_DATA_T] << std::setw(2) << " " << -h_Motion_Parameters[t + 0*EPI_DATA_T] << std::setw(2) << " " << -h_Motion_Parameters[t + 1*EPI_DATA_T] << std::endl;
    	    }
    	    motion.close();
    	}
    	else
    	{
    	    printf("Could not open %s for writing!\n",filenameWithExtension);
    	}
		free(filenameWithExtension);
	}

    //----------------------------
    // Write aligned data
    //----------------------------
    
	if (PRINT)
	{
		printf("Writing results to file\n");
	}

    // Create new nifti image
    nifti_image *outputNiftiT1 = nifti_copy_nim_info(inputMNI);
    allNiftiImages[numberOfNiftiImages] = outputNiftiT1;
	numberOfNiftiImages++;
    
	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftiT1, outputFilename, 0, 1);

	    if (WRITE_INTERPOLATED_T1)
		{
   			WriteNifti(outputNiftiT1,h_Interpolated_T1_Volume,"_t1_interpolated",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
   		if (WRITE_ALIGNED_T1_MNI_LINEAR)
		{
   			WriteNifti(outputNiftiT1,h_Aligned_T1_Volume_Linear,"_t1_aligned_mni_linear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
   		if (WRITE_ALIGNED_T1_MNI_NONLINEAR)
		{
   			WriteNifti(outputNiftiT1,h_Aligned_T1_Volume_NonLinear,"_t1_aligned_mni_nonlinear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}
	else
	{
	    nifti_set_filenames(outputNiftiT1, inputT1->fname, 0, 1);

	    if (WRITE_INTERPOLATED_T1)
		{
   			WriteNifti(outputNiftiT1,h_Interpolated_T1_Volume,"_interpolated",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
   		if (WRITE_ALIGNED_T1_MNI_LINEAR)
		{
   			WriteNifti(outputNiftiT1,h_Aligned_T1_Volume_Linear,"_aligned_mni_linear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
   		if (WRITE_ALIGNED_T1_MNI_NONLINEAR)
		{
   			WriteNifti(outputNiftiT1,h_Aligned_T1_Volume_NonLinear,"_aligned_mni_nonlinear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}



    // Create new nifti image
    nifti_image *outputNiftiEPI = nifti_copy_nim_info(inputMNI);
    allNiftiImages[numberOfNiftiImages] = outputNiftiEPI;
	numberOfNiftiImages++;

	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftiEPI, outputFilename, 0, 1);
	
		if (WRITE_ALIGNED_EPI_T1)
		{
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_T1,"_epi_aligned_t1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}    
		if (WRITE_ALIGNED_EPI_MNI)
		{
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_MNI_Linear,"_epi_aligned_mni_linear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_MNI_Nonlinear,"_epi_aligned_mni_nonlinear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}    
	}
	else
	{
	    nifti_set_filenames(outputNiftiEPI, inputfMRI->fname, 0, 1);

		if (WRITE_ALIGNED_EPI_T1)
		{
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_T1,"_aligned_t1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}    
		if (WRITE_ALIGNED_EPI_MNI)
		{
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_MNI_Linear,"_aligned_mni_linear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    	WriteNifti(outputNiftiEPI,h_Aligned_EPI_Volume_MNI_Nonlinear,"_aligned_mni_nonlinear",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}    
	}

    //----------------------------
    // Write preprocessed data
    //----------------------------
    
    // Create new nifti image	
    nifti_image *outputNiftifMRI = nifti_copy_nim_info(inputfMRI);
	outputNiftifMRI->nt = EPI_DATA_T;
    outputNiftifMRI->dim[4] = EPI_DATA_T;
    outputNiftifMRI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T;
    allNiftiImages[numberOfNiftiImages] = outputNiftifMRI;
	numberOfNiftiImages++;
    
	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftifMRI, outputFilename, 0, 1);
	}    

    if (WRITE_SLICETIMING_CORRECTED)
	{
    	WriteNifti(outputNiftifMRI,h_Slice_Timing_Corrected_fMRI_Volumes,"_slice_timing_corrected",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}
    if (WRITE_MOTION_CORRECTED)
	{
    	WriteNifti(outputNiftifMRI,h_Motion_Corrected_fMRI_Volumes,"_motion_corrected",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}
    if (WRITE_SMOOTHED)
	{
    	WriteNifti(outputNiftifMRI,h_Smoothed_fMRI_Volumes,"_smoothed",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}
    
    // Create new nifti image
    nifti_image *outputNiftifMRISingleVolume = nifti_copy_nim_info(inputfMRI);
    outputNiftifMRISingleVolume->nt = 1;	
    outputNiftifMRISingleVolume->dim[4] = 1;
    outputNiftifMRISingleVolume->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D;
    allNiftiImages[numberOfNiftiImages] = outputNiftifMRISingleVolume;
	numberOfNiftiImages++;

	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftifMRISingleVolume, outputFilename, 0, 1);

	    if (WRITE_EPI_MASK)
		{
    		WriteNifti(outputNiftifMRISingleVolume,h_EPI_Mask,"_epi_mask",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}
	else
	{
	    if (WRITE_EPI_MASK)
		{
    		WriteNifti(outputNiftifMRISingleVolume,h_EPI_Mask,"_mask",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}


    //------------------------------------------
    // Write statistical results, MNI space
    //------------------------------------------

   	// Create new nifti image
    nifti_image *outputNiftiStatisticsMNI = nifti_copy_nim_info(inputMNI);
    nifti_set_filenames(outputNiftiStatisticsMNI, inputfMRI->fname, 0, 1);
    allNiftiImages[numberOfNiftiImages] = outputNiftiStatisticsMNI;
	numberOfNiftiImages++;
    
	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftiStatisticsMNI, outputFilename, 0, 1);
	}

    if (WRITE_MNI_MASK)
	{
    	WriteNifti(outputNiftiStatisticsMNI,h_MNI_Mask,"_mask_mni",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}

    std::string beta = "_beta";
    std::string cope = "_cope";
    std::string tscores = "_tscores";
    std::string betaNoWhitening = "_beta_no_whitening";
    std::string copeNoWhitening = "_cope_no_whitening";
    std::string tscoresNoWhitening = "_tscores_no_whitening";
    std::string pvalues = "_pvalues";
    std::string PPM = "_PPM";
    std::string mni = "_MNI";
    std::string epi = "_EPI";
    std::string t1 = "_T1";    

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
	    if (!BAYESIAN)
	    {
			if (!WRITE_COMPACT)
			{
			    // Write each beta weight as a separate file
		        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
		        {
		            std::string temp = beta;
		            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_regressor000";
					}
					else if ((i+1) < 100)
					{
						ss << "_regressor00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_regressor0";
					}
					else
					{
						ss << "_regressor";
					}						
		            ss << i + 1;
		            temp.append(ss.str());
		            temp.append(mni);
		            WriteNifti(outputNiftiStatisticsMNI,&h_Beta_Volumes_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		        }
			    // Write each contrast volume as a separate file
		        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
		        {
		            std::string temp = cope;
		            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_contrast000";
					}
					else if ((i+1) < 100)
					{
						ss << "_contrast00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_contrast0";
					}
					else
					{
						ss << "_contrast";
					}						
		            ss << i + 1;
		            temp.append(ss.str());
		            temp.append(mni);
		            WriteNifti(outputNiftiStatisticsMNI,&h_Contrast_Volumes_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		        }  
				if (!BETAS_ONLY)
				{
			        // Write each t-map as a separate file
			        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			        {
						// nifti file contains t-scores
						outputNiftiStatisticsMNI->intent_code = 3;
		
			            std::string temp = tscores;
			            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
			            ss << i + 1;
			            temp.append(ss.str());
			            temp.append(mni);
			            WriteNifti(outputNiftiStatisticsMNI,&h_Statistical_Maps_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			        }
				}
			}
			else
			{
    			outputNiftiStatisticsMNI->ndim = 4;
			    outputNiftiStatisticsMNI->dim[0] = 4;

				// Write all beta weights as a single file
			    outputNiftiStatisticsMNI->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
				outputNiftiStatisticsMNI->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
			    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
	            std::string temp = beta;
	            temp.append("_allregressors");
	            temp.append(mni);
				WriteNifti(outputNiftiStatisticsMNI,h_Beta_Volumes_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

				// Write all contrast volumes as a single file
			    outputNiftiStatisticsMNI->nt = NUMBER_OF_CONTRASTS;
				outputNiftiStatisticsMNI->dim[4] = NUMBER_OF_CONTRASTS;
			    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_CONTRASTS;
	            temp = cope;
	            temp.append("_allcontrasts");
	            temp.append(mni);
				WriteNifti(outputNiftiStatisticsMNI,h_Contrast_Volumes_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

				if (!BETAS_ONLY)
				{
					// Write all statistical maps as a single file
					// nifti file contains t-scores
					outputNiftiStatisticsMNI->intent_code = 3;
		            temp = tscores;
		            temp.append("_allcontrasts");
		            temp.append(mni);
					WriteNifti(outputNiftiStatisticsMNI,h_Statistical_Maps_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
				}				
			}

			// No whitening

			if (WRITE_UNWHITENED_RESULTS && !BETAS_ONLY)
			{
				if (!WRITE_COMPACT)
				{
	    			outputNiftiStatisticsMNI->ndim = 3;
				    outputNiftiStatisticsMNI->dim[0] = 3;
				    outputNiftiStatisticsMNI->nt = 1;
					outputNiftiStatisticsMNI->dim[4] = 1;
				    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D;

					// Write each beta weight as a separate file
			        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
			        {
			            std::string temp = betaNoWhitening;
			            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_regressor000";
						}
						else if ((i+1) < 100)
						{
							ss << "_regressor00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_regressor0";
						}
						else
						{
							ss << "_regressor";
						}						
			            ss << i + 1;
			            temp.append(ss.str());
			            temp.append(mni);
			            WriteNifti(outputNiftiStatisticsMNI,&h_Beta_Volumes_No_Whitening_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			        }
				    // Write each contrast volume as a separate file
			        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			        {
			            std::string temp = copeNoWhitening;
			            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
			            ss << i + 1;
			            temp.append(ss.str());
			            temp.append(mni);
			            WriteNifti(outputNiftiStatisticsMNI,&h_Contrast_Volumes_No_Whitening_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			        }  
		    	    // Write each t-map as a separate file
			        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			        {
			            std::string temp = tscoresNoWhitening;
			            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
			            ss << i + 1;
			            temp.append(ss.str());
			            temp.append(mni);
			            WriteNifti(outputNiftiStatisticsMNI,&h_Statistical_Maps_No_Whitening_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			        }
				}
				else
				{
	    			outputNiftiStatisticsMNI->ndim = 4;
				    outputNiftiStatisticsMNI->dim[0] = 4;

					// Write all beta weights as a single file
				    outputNiftiStatisticsMNI->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
					outputNiftiStatisticsMNI->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
				    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
		            std::string temp = betaNoWhitening;
		            temp.append("_allregressors");
		            temp.append(mni);
					WriteNifti(outputNiftiStatisticsMNI,h_Beta_Volumes_No_Whitening_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
					// Write all contrast volumes as a single file
				    outputNiftiStatisticsMNI->nt = NUMBER_OF_CONTRASTS;
					outputNiftiStatisticsMNI->dim[4] = NUMBER_OF_CONTRASTS;
				    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_CONTRASTS;
		            temp = copeNoWhitening;
		            temp.append("_allcontrasts");
		            temp.append(mni);
					WriteNifti(outputNiftiStatisticsMNI,h_Contrast_Volumes_No_Whitening_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
					if (!BETAS_ONLY)
					{
						// Write all statistical maps as a single file
						// nifti file contains t-scores
						outputNiftiStatisticsMNI->intent_code = 3;
			            temp = tscoresNoWhitening;
			            temp.append("_allcontrasts");
			            temp.append(mni);
						WriteNifti(outputNiftiStatisticsMNI,h_Statistical_Maps_No_Whitening_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
					}
				}
			}
	        if (PERMUTE && !BETAS_ONLY)
	        {
				// nifti file contains p-values
				outputNiftiStatisticsMNI->intent_code = 22;

				if (!WRITE_COMPACT)
				{
	    			outputNiftiStatisticsMNI->ndim = 3;
				    outputNiftiStatisticsMNI->dim[0] = 3;
				    outputNiftiStatisticsMNI->nt = 1;
					outputNiftiStatisticsMNI->dim[4] = 1;
				    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D;

		            // Write each p-map as a separate file
		            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
		            {
		                std::string temp = pvalues;
				        std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
		                ss << i + 1;
		                temp.append(ss.str());
		                temp.append(mni);
		                WriteNifti(outputNiftiStatisticsMNI,&h_P_Values_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		            }
				}
				else
				{
					// Write all p-maps as a single file
	    			outputNiftiStatisticsMNI->ndim = 4;
				    outputNiftiStatisticsMNI->dim[0] = 4;
				    outputNiftiStatisticsMNI->nt = NUMBER_OF_CONTRASTS;
					outputNiftiStatisticsMNI->dim[4] = NUMBER_OF_CONTRASTS;
				    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * NUMBER_OF_CONTRASTS;
		            std::string temp = pvalues;
		            temp.append("_allcontrasts");
		            temp.append(mni);
					WriteNifti(outputNiftiStatisticsMNI,h_P_Values_MNI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
				}
	        }
	    }
	    else if (BAYESIAN)
	    {
		    // Write each beta weight as a separate file
	        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
	        {
	            std::string temp = beta;
	            std::stringstream ss;
				if ((i+1) < 10)
				{
   		            ss << "_regressor000";
				}
				else if ((i+1) < 100)
				{
					ss << "_regressor00";
				}
				else if ((i+1) < 1000)
				{
					ss << "_regressor0";
				}
				else
				{
					ss << "_regressor";
				}						
	            ss << i + 1;
	            temp.append(ss.str());
	            temp.append(mni);
	            WriteNifti(outputNiftiStatisticsMNI,&h_Beta_Volumes_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	        }
	        // Write each PPM as a separate file
	        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
	        {
	            std::string temp = PPM;
	            std::stringstream ss;
				if ((i+1) < 10)
				{
   		            ss << "_contrast000";
				}
				else if ((i+1) < 100)
				{
					ss << "_contrast00";
				}
				else if ((i+1) < 1000)
				{
					ss << "_contrast0";
				}
				else
				{
					ss << "_contrast";
				}						
	            ss << i + 1;
	            temp.append(ss.str());
	            temp.append(mni);
	            WriteNifti(outputNiftiStatisticsMNI,&h_Statistical_Maps_MNI[i * MNI_DATA_W * MNI_DATA_H * MNI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	        }
	    }

	    if (WRITE_AR_ESTIMATES_MNI && !BAYESIAN && !BETAS_ONLY)
	    {
	        WriteNifti(outputNiftiStatisticsMNI,h_AR1_Estimates_MNI,"_ar1_estimates_MNI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	        WriteNifti(outputNiftiStatisticsMNI,h_AR2_Estimates_MNI,"_ar2_estimates_MNI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	        WriteNifti(outputNiftiStatisticsMNI,h_AR3_Estimates_MNI,"_ar3_estimates_MNI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	        WriteNifti(outputNiftiStatisticsMNI,h_AR4_Estimates_MNI,"_ar4_estimates_MNI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    }
		else if (WRITE_AR_ESTIMATES_MNI && BAYESIAN)
		{
	        WriteNifti(outputNiftiStatisticsMNI,h_AR1_Estimates_MNI,"_ar1_estimates_MNI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}		
	}
	else if (!BETAS_ONLY && REGRESS_ONLY)
	{		
		outputNiftiStatisticsMNI->nt = EPI_DATA_T;
		outputNiftiStatisticsMNI->ndim = 4;
		outputNiftiStatisticsMNI->dim[0] = 4;
	    outputNiftiStatisticsMNI->dim[4] = EPI_DATA_T;
	    outputNiftiStatisticsMNI->pixdim[6] = inputfMRI->pixdim[6];
	    outputNiftiStatisticsMNI->pixdim[7] = inputfMRI->pixdim[7];
	    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * EPI_DATA_T;
		outputNiftiStatisticsMNI->dt = TR;		
	    
		WriteNifti(outputNiftiStatisticsMNI,h_Residuals_MNI,"_residuals_mni",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}
	else if (PREPROCESSING_ONLY)
	{		
		outputNiftiStatisticsMNI->nt = EPI_DATA_T;
		outputNiftiStatisticsMNI->ndim = 4;
		outputNiftiStatisticsMNI->dim[0] = 4;
	    outputNiftiStatisticsMNI->dim[4] = EPI_DATA_T;
	    outputNiftiStatisticsMNI->pixdim[6] = inputfMRI->pixdim[6];
	    outputNiftiStatisticsMNI->pixdim[7] = inputfMRI->pixdim[7];
	    outputNiftiStatisticsMNI->nvox = MNI_DATA_W * MNI_DATA_H * MNI_DATA_D * EPI_DATA_T;
		outputNiftiStatisticsMNI->dt = TR;		
	    
		WriteNifti(outputNiftiStatisticsMNI,h_fMRI_Volumes_MNI,"_preprocessed_mni",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	}


    //------------------------------------------
    // Write statistical results, EPI space
    //------------------------------------------
    
    // Create new nifti image
    nifti_image *outputNiftiStatisticsEPI = nifti_copy_nim_info(inputfMRI);
    outputNiftiStatisticsEPI->nt = 1;
    outputNiftiStatisticsEPI->ndim = 3;
    outputNiftiStatisticsEPI->dim[0] = 3;
    outputNiftiStatisticsEPI->dim[4] = 1;
	outputNiftiStatisticsEPI->scl_slope = 1;	
    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D;
	nifti_free_extensions(outputNiftiStatisticsEPI);
    allNiftiImages[numberOfNiftiImages] = outputNiftiStatisticsEPI;
    numberOfNiftiImages++;
    
	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftiStatisticsEPI, outputFilename, 0, 1);
	}

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{
	    if (WRITE_ACTIVITY_EPI)
	    {
    	    if (!BAYESIAN)
    	    {
				if (!WRITE_COMPACT)
				{
	    	        // Write each beta weight as a separate file
	    	        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
	    	        {
    		            std::string temp = beta;
    		            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_regressor000";
						}
						else if ((i+1) < 100)
						{
							ss << "_regressor00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_regressor0";
						}
						else
						{
							ss << "_regressor";
						}									

						ss << i + 1;
    		            temp.append(ss.str());
    		            temp.append(epi);
    		            WriteNifti(outputNiftiStatisticsEPI,&h_Beta_Volumes_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    		        }
    		        // Write each contrast volume as a separate file
    		        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
    		        {
    		            std::string temp = cope;
    		            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
    		            ss << i + 1;
    		            temp.append(ss.str());
    		            temp.append(epi);
    		            WriteNifti(outputNiftiStatisticsEPI,&h_Contrast_Volumes_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    		        }
					if (!BETAS_ONLY)
					{
		    	        // Write each t-map as a separate file
		    	        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
		    	        {
							// nifti file contains t-scores
							outputNiftiStatisticsEPI->intent_code = 3;
	
    			            std::string temp = tscores;
    			            std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						 
	   			            ss << i + 1;
    			            temp.append(ss.str());
    			            temp.append(epi);
    			            WriteNifti(outputNiftiStatisticsEPI,&h_Statistical_Maps_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    			        }
					}
				}
				else
				{
	    			outputNiftiStatisticsEPI->ndim = 4;
				    outputNiftiStatisticsEPI->dim[0] = 4;

					// Write all beta weights as a single file
				    outputNiftiStatisticsEPI->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
					outputNiftiStatisticsEPI->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
				    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
		            std::string temp = beta;
		            temp.append("_allregressors");
		            temp.append(epi);
					WriteNifti(outputNiftiStatisticsEPI,h_Beta_Volumes_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

					// Write all contrast volumes as a single file
				    outputNiftiStatisticsEPI->nt = NUMBER_OF_CONTRASTS;
					outputNiftiStatisticsEPI->dim[4] = NUMBER_OF_CONTRASTS;
				    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_CONTRASTS;
		            temp = cope;
		            temp.append("_allcontrasts");
		            temp.append(epi);
					WriteNifti(outputNiftiStatisticsEPI,h_Contrast_Volumes_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

					if (!BETAS_ONLY)
					{
						// Write all statistical maps as a single file
						// nifti file contains t-scores
						outputNiftiStatisticsEPI->intent_code = 3;
			            temp = tscores;
			            temp.append("_allcontrasts");
			            temp.append(epi);
						WriteNifti(outputNiftiStatisticsEPI,h_Statistical_Maps_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
					}
				}
	
				// No whitening
	
				if (WRITE_UNWHITENED_RESULTS && !BETAS_ONLY)
				{
					if (!WRITE_COMPACT)
					{
		    			outputNiftiStatisticsEPI->ndim = 3;
					    outputNiftiStatisticsEPI->dim[0] = 3;
					    outputNiftiStatisticsEPI->nt = 1;
						outputNiftiStatisticsEPI->dim[4] = 1;
					    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D;									

						// Write each beta weight as a separate file
	    		        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
			            {
			                std::string temp = betaNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_regressor000";
							}
							else if ((i+1) < 100)
							{
								ss << "_regressor00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_regressor0";
							}
							else
							{
								ss << "_regressor";
							}						
							ss << i + 1;
			                temp.append(ss.str());
			                temp.append(epi);
			                WriteNifti(outputNiftiStatisticsEPI,&h_Beta_Volumes_No_Whitening_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
			            // Write each contrast volume as a separate file
			            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			            {
			                std::string temp = copeNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
			                ss << i + 1;
			                temp.append(ss.str());
			                temp.append(epi);
			                WriteNifti(outputNiftiStatisticsEPI,&h_Contrast_Volumes_No_Whitening_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
			            // Write each t-map as a separate file
			            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			            {
			                std::string temp = tscoresNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
			                ss << i + 1;
			                temp.append(ss.str());
			                temp.append(epi);
			                WriteNifti(outputNiftiStatisticsEPI,&h_Statistical_Maps_No_Whitening_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
					}
					else
					{
		    			outputNiftiStatisticsEPI->ndim = 4;
					    outputNiftiStatisticsEPI->dim[0] = 4;

						// Write all beta weights as a single file
					    outputNiftiStatisticsEPI->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
						outputNiftiStatisticsEPI->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
					    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
			            std::string temp = betaNoWhitening;
			            temp.append("_allregressors");
			            temp.append(epi);
						WriteNifti(outputNiftiStatisticsEPI,h_Beta_Volumes_No_Whitening_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
						// Write all contrast volumes as a single file
					    outputNiftiStatisticsEPI->nt = NUMBER_OF_CONTRASTS;
						outputNiftiStatisticsEPI->dim[4] = NUMBER_OF_CONTRASTS;
					    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_CONTRASTS;
			            temp = copeNoWhitening;
			            temp.append("_allcontrasts");
			            temp.append(epi);
						WriteNifti(outputNiftiStatisticsEPI,h_Contrast_Volumes_No_Whitening_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
						if (!BETAS_ONLY)
						{
							// Write all statistical maps as a single file
							// nifti file contains t-scores
							outputNiftiStatisticsEPI->intent_code = 3;
				            temp = tscoresNoWhitening;
				            temp.append("_allcontrasts");
				            temp.append(epi);
							WriteNifti(outputNiftiStatisticsEPI,h_Statistical_Maps_No_Whitening_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
						}
					}
				}
	
    	        if (PERMUTE && !BETAS_ONLY)
    	        {
					// nifti file contains p-values
					outputNiftiStatisticsEPI->intent_code = 22;

					if (!WRITE_COMPACT)
					{
		    			outputNiftiStatisticsEPI->ndim = 3;
					    outputNiftiStatisticsEPI->dim[0] = 3;
					    outputNiftiStatisticsEPI->nt = 1;
						outputNiftiStatisticsEPI->dim[4] = 1;
					    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D;
	
			            // Write each p-map as a separate file
			            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			            {
			                std::string temp = pvalues;
					        std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
			                ss << i + 1;
			                temp.append(ss.str());
			                temp.append(epi);
			                WriteNifti(outputNiftiStatisticsEPI,&h_P_Values_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
					}
					else
					{
						// Write all p-maps as a single file
		    			outputNiftiStatisticsEPI->ndim = 4;
					    outputNiftiStatisticsEPI->dim[0] = 4;
					    outputNiftiStatisticsEPI->nt = NUMBER_OF_CONTRASTS;
						outputNiftiStatisticsEPI->dim[4] = NUMBER_OF_CONTRASTS;
					    outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * NUMBER_OF_CONTRASTS;
			            std::string temp = pvalues;
			            temp.append("_allcontrasts");
			            temp.append(epi);
						WriteNifti(outputNiftiStatisticsEPI,h_P_Values_EPI,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
					}
    	        }
    	    }
    	    else if (BAYESIAN)
    	    {
    	        // Write each beta weight as a separate file
    	        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
    	        {
    	            std::string temp = beta;
    	            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_regressor000";
					}
					else if ((i+1) < 100)
					{
						ss << "_regressor00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_regressor0";
					}
					else
					{
						ss << "_regressor";
					}						
					ss << i + 1;
    	            temp.append(ss.str());
    	            temp.append(epi);
    	            WriteNifti(outputNiftiStatisticsEPI,&h_Beta_Volumes_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	        }
    	        // Write each PPM as a separate file
    	        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
    	        {
    	            std::string temp = PPM;
    	            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_contrast000";
					}
					else if ((i+1) < 100)
					{
						ss << "_contrast00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_contrast0";
					}
					else
					{
						ss << "_contrast";
					}						
    	            ss << i + 1;
    	            temp.append(ss.str());
    	            temp.append(epi);
    	            WriteNifti(outputNiftiStatisticsEPI,&h_Statistical_Maps_EPI[i * EPI_DATA_W * EPI_DATA_H * EPI_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	        }
    	    }
    	}
    
		outputNiftiStatisticsEPI->intent_code = 0;

    	if (WRITE_AR_ESTIMATES_EPI && !BAYESIAN && !BETAS_ONLY)
    	{
    	    WriteNifti(outputNiftiStatisticsEPI,h_AR1_Estimates_EPI,"_ar1_estimates_EPI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	    WriteNifti(outputNiftiStatisticsEPI,h_AR2_Estimates_EPI,"_ar2_estimates_EPI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	    WriteNifti(outputNiftiStatisticsEPI,h_AR3_Estimates_EPI,"_ar3_estimates_EPI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	    WriteNifti(outputNiftiStatisticsEPI,h_AR4_Estimates_EPI,"_ar4_estimates_EPI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);	
    	}    
    	else if (WRITE_AR_ESTIMATES_EPI && BAYESIAN)
    	{
    	    WriteNifti(outputNiftiStatisticsEPI,h_AR1_Estimates_EPI,"_ar1_estimates_EPI",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	}    

		if (WRITE_RESIDUALS_EPI && !BAYESIAN && !BETAS_ONLY)
		{
		    outputNiftiStatisticsEPI->ndim = 4;
			outputNiftiStatisticsEPI->nt = EPI_DATA_T;
			outputNiftiStatisticsEPI->dim[0] = 4;
	    	outputNiftiStatisticsEPI->dim[4] = EPI_DATA_T;
	    	outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T;
			WriteNifti(outputNiftiStatisticsEPI,h_Residuals_EPI,"_residuals",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}
	else if (REGRESS_ONLY)
	{
		if (WRITE_ACTIVITY_EPI)
	    {
			outputNiftiStatisticsEPI->ndim = 4;
	    	outputNiftiStatisticsEPI->nt = EPI_DATA_T;
			outputNiftiStatisticsEPI->dim[0] = 4;
	    	outputNiftiStatisticsEPI->dim[4] = EPI_DATA_T;
	    	outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T;
			WriteNifti(outputNiftiStatisticsEPI,h_Residuals_EPI,"_residuals",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}
	else if (PREPROCESSING_ONLY)
	{
		if (WRITE_ACTIVITY_EPI)
	    {
			outputNiftiStatisticsEPI->ndim = 4;
	    	outputNiftiStatisticsEPI->nt = EPI_DATA_T;
			outputNiftiStatisticsEPI->dim[0] = 4;
	    	outputNiftiStatisticsEPI->dim[4] = EPI_DATA_T;
	    	outputNiftiStatisticsEPI->nvox = EPI_DATA_W * EPI_DATA_H * EPI_DATA_D * EPI_DATA_T;
			WriteNifti(outputNiftiStatisticsEPI,h_fMRI_Volumes,"_preprocessed",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
		}
	}



    //------------------------------------------
    // Write statistical results, T1 space
    //------------------------------------------
    
    // Create new nifti image
    nifti_image *outputNiftiStatisticsT1 = nifti_copy_nim_info(inputT1);
	nifti_set_filenames(outputNiftiStatisticsT1, inputfMRI->fname, DONT_CHECK_EXISTING_FILE, 1);
    outputNiftiStatisticsT1->nt = 1;
    outputNiftiStatisticsT1->dim[0] = 3;
    outputNiftiStatisticsT1->dim[4] = 1;
	outputNiftiStatisticsT1->scl_slope = 1;	
    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D;
	nifti_free_extensions(outputNiftiStatisticsT1);
    allNiftiImages[numberOfNiftiImages] = outputNiftiStatisticsT1;
    numberOfNiftiImages++;

	if (CHANGE_OUTPUT_FILENAME)
	{
		nifti_set_filenames(outputNiftiStatisticsT1, outputFilename, 0, 1);
	}

	if (!REGRESS_ONLY && !PREPROCESSING_ONLY)
	{  
	    if (WRITE_ACTIVITY_T1)
	    {
    	    if (!BAYESIAN)
    	    {
				if (!WRITE_COMPACT)
				{
	    	        // Write each beta weight as a separate file
    		        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
    		        {
    		            std::string temp = beta;
    		            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_regressor000";
						}
						else if ((i+1) < 100)
						{
							ss << "_regressor00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_regressor0";
						}
						else
						{
							ss << "_regressor";
						}						
						ss << i + 1;
    		            temp.append(ss.str());
    		            temp.append(t1);
    		            WriteNifti(outputNiftiStatisticsT1,&h_Beta_Volumes_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    		        }
    		        // Write each contrast volume as a separate file
    		        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
    		        {
    		            std::string temp = cope;
    		            std::stringstream ss;
						if ((i+1) < 10)
						{
	    		            ss << "_contrast000";
						}
						else if ((i+1) < 100)
						{
							ss << "_contrast00";
						}
						else if ((i+1) < 1000)
						{
							ss << "_contrast0";
						}
						else
						{
							ss << "_contrast";
						}						
    		            ss << i + 1;
    		            temp.append(ss.str());
    		            temp.append(t1);
    		            WriteNifti(outputNiftiStatisticsT1,&h_Contrast_Volumes_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    		        }
					if (!BETAS_ONLY)
					{
		    	        // Write each t-map as a separate file
		    	        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
		    	        {
		    	            std::string temp = tscores;
		    	            std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
		    	            ss << i + 1;
		    	            temp.append(ss.str());
		    	            temp.append(t1);
		    	            WriteNifti(outputNiftiStatisticsT1,&h_Statistical_Maps_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    			        }
					}
				}
				else
				{
					outputNiftiStatisticsT1->ndim = 4;
				    outputNiftiStatisticsT1->dim[0] = 4;

					// Write all beta weights as a single file
				    outputNiftiStatisticsT1->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
					outputNiftiStatisticsT1->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
				    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
		            std::string temp = beta;
		            temp.append("_allregressors");
		            temp.append(t1);
					WriteNifti(outputNiftiStatisticsT1,h_Beta_Volumes_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

					// Write all contrast volumes as a single file
				    outputNiftiStatisticsT1->nt = NUMBER_OF_CONTRASTS;
					outputNiftiStatisticsT1->dim[4] = NUMBER_OF_CONTRASTS;
				    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_CONTRASTS;
		            temp = cope;
		            temp.append("_allcontrasts");
		            temp.append(t1);
					WriteNifti(outputNiftiStatisticsT1,h_Contrast_Volumes_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);

					if (!BETAS_ONLY)
					{
						// Write all statistical maps as a single file
						// nifti file contains t-scores
						outputNiftiStatisticsT1->intent_code = 3;
			            temp = tscores;
			            temp.append("_allcontrasts");
			            temp.append(t1);
						WriteNifti(outputNiftiStatisticsT1,h_Statistical_Maps_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
					}
				}

			    if (WRITE_AR_ESTIMATES_T1 && !BETAS_ONLY)
			    {
	    		    WriteNifti(outputNiftiStatisticsT1,h_AR1_Estimates_T1,"_ar1_estimates_T1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    		    WriteNifti(outputNiftiStatisticsT1,h_AR2_Estimates_T1,"_ar2_estimates_T1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    		    WriteNifti(outputNiftiStatisticsT1,h_AR3_Estimates_T1,"_ar3_estimates_T1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    		    WriteNifti(outputNiftiStatisticsT1,h_AR4_Estimates_T1,"_ar4_estimates_T1",ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	    		}

				// No whitening
	
				if (WRITE_UNWHITENED_RESULTS && !BETAS_ONLY)
				{
					if (!WRITE_COMPACT)
					{
		    			outputNiftiStatisticsT1->ndim = 3;
					    outputNiftiStatisticsT1->dim[0] = 3;
					    outputNiftiStatisticsT1->nt = 1;
						outputNiftiStatisticsT1->dim[4] = 1;
					    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D;	

						// Write each beta weight as a separate file
    			        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
			            {
			                std::string temp = betaNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_regressor000";
							}
							else if ((i+1) < 100)
							{
								ss << "_regressor00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_regressor0";
							}
							else
							{
								ss << "_regressor";
							}						
							ss << i + 1;
			                temp.append(ss.str());
			                temp.append(t1);
			                WriteNifti(outputNiftiStatisticsT1,&h_Beta_Volumes_No_Whitening_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
			            // Write each contrast volume as a separate file
			            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			            {
			                std::string temp = copeNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
			                ss << i + 1;
			                temp.append(ss.str());
			                temp.append(t1);
			                WriteNifti(outputNiftiStatisticsT1,&h_Contrast_Volumes_No_Whitening_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
			            // Write each t-map as a separate file
			            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
			            {
			                std::string temp = tscoresNoWhitening;
			                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
			                ss << i + 1;
			                temp.append(ss.str());
			                temp.append(t1);
			                WriteNifti(outputNiftiStatisticsT1,&h_Statistical_Maps_No_Whitening_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
			            }
					}
					else
					{
						outputNiftiStatisticsT1->ndim = 4;
					    outputNiftiStatisticsT1->dim[0] = 4;

						// Write all beta weights as a single file
					    outputNiftiStatisticsT1->nt = NUMBER_OF_TOTAL_GLM_REGRESSORS;
						outputNiftiStatisticsT1->dim[4] = NUMBER_OF_TOTAL_GLM_REGRESSORS;
					    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_TOTAL_GLM_REGRESSORS;
			            std::string temp = betaNoWhitening;
			            temp.append("_allregressors");
			            temp.append(t1);
						WriteNifti(outputNiftiStatisticsT1,h_Beta_Volumes_No_Whitening_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
						// Write all contrast volumes as a single file
					    outputNiftiStatisticsT1->nt = NUMBER_OF_CONTRASTS;
						outputNiftiStatisticsT1->dim[4] = NUMBER_OF_CONTRASTS;
					    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_CONTRASTS;
			            temp = copeNoWhitening;
			            temp.append("_allcontrasts");
			            temp.append(epi);
						WriteNifti(outputNiftiStatisticsT1,h_Contrast_Volumes_No_Whitening_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
	
						if (!BETAS_ONLY)
						{
							// Write all statistical maps as a single file
							// nifti file contains t-scores
							outputNiftiStatisticsT1->intent_code = 3;
				            temp = tscoresNoWhitening;
				            temp.append("_allcontrasts");
				            temp.append(t1);
							WriteNifti(outputNiftiStatisticsT1,h_Statistical_Maps_No_Whitening_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
						}
					}
				}
	
    	        if (PERMUTE && !BETAS_ONLY)
    	        {
					if (!WRITE_COMPACT)
					{
		    			outputNiftiStatisticsT1->ndim = 3;
					    outputNiftiStatisticsT1->dim[0] = 3;
					    outputNiftiStatisticsT1->nt = 1;
						outputNiftiStatisticsT1->dim[4] = 1;
					    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D;

	    	            // Write each p-map as a separate file
    		            for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
    		            {
    		                std::string temp = pvalues;
    		                std::stringstream ss;
							if ((i+1) < 10)
							{
		    		            ss << "_contrast000";
							}
							else if ((i+1) < 100)
							{
								ss << "_contrast00";
							}
							else if ((i+1) < 1000)
							{
								ss << "_contrast0";
							}
							else
							{
								ss << "_contrast";
							}						
    		                ss << i + 1;
    		                temp.append(ss.str());
    		                temp.append(t1);
    		                WriteNifti(outputNiftiStatisticsT1,&h_P_Values_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    		            }
					}
					else
					{
						// Write all p-maps as a single file
		    			outputNiftiStatisticsT1->ndim = 4;
					    outputNiftiStatisticsT1->dim[0] = 4;
					    outputNiftiStatisticsT1->nt = NUMBER_OF_CONTRASTS;
						outputNiftiStatisticsT1->dim[4] = NUMBER_OF_CONTRASTS;
					    outputNiftiStatisticsT1->nvox = T1_DATA_W * T1_DATA_H * T1_DATA_D * NUMBER_OF_CONTRASTS;
			            std::string temp = pvalues;
			            temp.append("_allcontrasts");
			            temp.append(t1);
						WriteNifti(outputNiftiStatisticsT1,h_P_Values_T1,temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
					}
    	        }
    	    }
    	    else if (BAYESIAN)
    	    {
    	        // Write each beta weight as a separate file
    	        for (size_t i = 0; i < NUMBER_OF_TOTAL_GLM_REGRESSORS; i++)
    	        {
    	            std::string temp = beta;
    	            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_regressor000";
					}
					else if ((i+1) < 100)
					{
						ss << "_regressor00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_regressor0";
					}
					else
					{
						ss << "_regressor";
					}						
					ss << i + 1;
    	            temp.append(ss.str());
    	            temp.append(t1);
    	            WriteNifti(outputNiftiStatisticsT1,&h_Beta_Volumes_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	        }
    	        // Write each PPM as a separate file
    	        for (size_t i = 0; i < NUMBER_OF_CONTRASTS; i++)
    	        {
    	            std::string temp = PPM;
    	            std::stringstream ss;
					if ((i+1) < 10)
					{
    		            ss << "_contrast000";
					}
					else if ((i+1) < 100)
					{
						ss << "_contrast00";
					}
					else if ((i+1) < 1000)
					{
						ss << "_contrast0";
					}
					else
					{
						ss << "_contrast";
					}						
    	            ss << i + 1;
    	            temp.append(ss.str());
    	            temp.append(t1);
    	            WriteNifti(outputNiftiStatisticsT1,&h_Statistical_Maps_T1[i * T1_DATA_W * T1_DATA_H * T1_DATA_D],temp.c_str(),ADD_FILENAME,DONT_CHECK_EXISTING_FILE);
    	        }
    	    }
    	}
	}
   
    endTime = GetWallTime();
    
	if (VERBOS)
	{
		printf("It took %f seconds to write the nifti files\n",(float)(endTime - startTime));
	}  
    
    // Free all memory
    FreeAllMemory(allMemoryPointers,numberOfMemoryPointers);
    FreeAllNiftiImages(allNiftiImages,numberOfNiftiImages);

	free(EPI_DATA_T_PER_RUN);
    
    return EXIT_SUCCESS;
}



