/******************************************************************************
 *
 *
	The following code describes a simple example 
	on how to use the Dirac Encoder API.  For a more 
	detailed usage refer to the sample encode application, 
	encoder/dirac_encoder.cpp, in the Dirac distribution. 
	The following sample reads uncompressed Standard Definition 
	format digital data (SD576) in Planar YUV 4:2:0 format from 
	an input file, and writes the encoded bitstream to an output file.
 *
 *	From 
 * http://dirac.sourceforge.net/documentation/code/programmers_guide/encoder_api_example.htm
 *
 * %Id:%
 *****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include "pthread.h"

#if defined (PROC_THREADS)	// covers whole file

/* Include Dirac Encoder Header file */
#include "libdirac_encoder/dirac_encoder.h"
#include "threadedProc.h"
#include "processor.h"

#define ENCBUF_SIZE (1024*100)

extern int run_dirac_encode (int argc, char **argv)
{
    FILE *in;
    FILE *out;
    dirac_encoder_context_t enc_ctx;
    dirac_encoder_t *encoder;
    unsigned char *unc_frame;
    int unc_frame_size;
    unsigned char enc_frame[ENCBUF_SIZE];

    if (argc < 3)
    {
        Log (0,"Usage : %s input-file output-file\n", argv[0]);
        return(1);
    }
    Log (0, "Encoder got : %s input %s output %s\n", argv[0],argv[1],argv[2]);

    /* open input file */
    if ( (in = fopen (argv[1], "rb")) == NULL)
    {
        Log (0,"Encoder bad read file %s, %s\n", argv[1], strerror(errno));
        return (errno);
    }

    /* open output file */
    if ( (out = fopen (argv[2], "wb")) == NULL)
    {
        Log (0,"Encoder bad write file %s, %s\n", argv[2], strerror(errno));
        return (errno);
    }

    /* Initialise the encoder context with presets for SD576 */
    dirac_encoder_context_init (&enc_ctx, SD576);
    
    /* override some of the preset defaults */
    /* Set quality factor to 7.5 */
    enc_ctx.enc_params.qf = 7.5;
    
    /* set input video type to progressive video */
    enc_ctx.seq_params.interlace = 0;
    
    /* set flag to retrieve locally decoded frames from encoder */
    enc_ctx.decode_flag = 1;
    
    /* Initialise the encoder with the encoder context */
    if ( (encoder = dirac_encoder_init(&enc_ctx, 0)) == NULL)
    {
        Log (0,"Error initialising the encoder\n");
        return (-1);
    }

    /* Calculate the size of an uncompressed frame */
    unc_frame_size = (encoder->enc_ctx.seq_params.height * 
       encoder->enc_ctx.seq_params.width) +
       2*(encoder->enc_ctx.seq_params.chroma_height 
               * encoder->enc_ctx.seq_params.chroma_width);

    /* Allocate memory for an uncompressed frame */
    unc_frame = (unsigned char *)malloc(unc_frame_size);

    /* Main loop */
    while (fread(unc_frame, unc_frame_size, 1, in) == 1)
    {
        /* 
        * Read one frame of uncompressed data into the buffer and
        * load it into the encoder
        */
        if (dirac_encoder_load (encoder, unc_frame, unc_frame_size) > 0)
        {
            dirac_encoder_state_t state;
            do
            { 
				/* (void) pthread_testcancel();	 */

                /*
                * Frame loaded successfully, so now set up the encode
                * buffer in the encoder handler before calling the
                * encoding function.
                */
                encoder->enc_buf.buffer = enc_frame;
                encoder->enc_buf.size = ENCBUF_SIZE;
                
                /* Call the encoding function */
                state = dirac_encoder_output (encoder);
                
                /*
                * Depending on the return value of the encode function
                * take appropriate action
                */
                switch (state)
                {
                case ENC_STATE_AVAIL:
                    /* 
                    * Encoded frame available in enc_buf. Write the
                    * buffer to the output file
                    */
                    fwrite (encoder->enc_buf.buffer, encoder->enc_buf.size,
                            1, out);
                    /* 
                    * In addition to the encoded frame, the following metadata
                    * is also available.
                    * encoded frame stats in encoder->enc_fstats
                    * encoded frame params in encoder->enc_fparams
                    */
                    break;

                case ENC_STATE_BUFFER:
                    /*
                    * Encoder needs more data to continue processing
                    */
                    break;

                case ENC_STATE_INVALID:
                default:
                    Log (0,"Irrecoverable error. quitting...");
                    free (unc_frame);
                    dirac_encoder_close(encoder);
                    return (-1);
                    break;
                }
                if (encoder->decoded_frame_avail)
                {
                    /*
                    * Locally decoded frame available in encoder->dec_buf
                    */
                }
                if (encoder->instr_data_avail)
                {
                    /*
                    * Instrumentation data available in encoder->instr
                    */
                }
            } while (state == ENC_STATE_AVAIL);
        }
    }

    Log (0,"Freeing encoder resources...\n");

    /* Retrieve end of sequence information */
    encoder->enc_buf.buffer = enc_frame;
    encoder->enc_buf.size = ENCBUF_SIZE;
    dirac_encoder_end_sequence( encoder );
    fwrite (encoder->enc_buf.buffer, encoder->enc_buf.size, 1, out);
    /* Sequence statistics available in encoder->enc_seqstats; */

    /* Free the encoder resources */
    dirac_encoder_close(encoder);
    
    /* Free the uncompressed data buffer */
    free (unc_frame);
    
    fclose (in);
    fclose (out);

	return 0;
}




/* 
 *		Thread entry point for encode 
 */

/* do this later....  something odd in the pthread_cleanup_push macro.. */
void dirac_thread_exit_msg (void * code)
{
	int *ecode = (int *)code;

	Log (0, "exiting DIRAC Encoder thread - exit code %d\n", *ecode);
}


extern unsigned long get_thrd_id (void);




void * dirac_encode_thrd (void * context) 
{
	int * continual = (int *) context;
	static int x  = 0;
	void *pusharg;
	int	  pushint = 666;
	char arg2 [100];
	char *args[4]  = {"dirac_encode", "../Test/vidframe.yuv"} ;

	sprintf (arg2, "../Test/vid_frame_out.%x", (int) get_thrd_id ());

	args[2] = arg2;
	args[3] = NULL;
	
	/* do push here if I can get it to work... */
	pusharg = (void *) &pushint;
	pthread_cleanup_push (dirac_thread_exit_msg, pusharg);	// broken

	/* do { */
		Log (0, "Running DIRAC Encoder thread %s\n", 
				(*continual) ? "- continuallly" : "");

		x = run_dirac_encode (3, args);

		/* (void) pthread_testcancel(); */

	/* } while (*continual); */

	Log (0, "exiting DIRAC Encoder thread - exit code %d\n", x);

	pthread_cleanup_pop (0);

	return (void *) &x;

}


#endif // PROC_THREADS


// endofile

