#include <math.h>

#include <rsf.h>

#include "split1.h"

void split1 (bool verb, bool inv, float eps,  
	     int nw, float dw, 
	     int nz, float dz,
	     int nx, float dx,
	     float **vt, float *v,
	     float complex **cp, float **q)
{
    int nk,iz,iw,ix,jx;
    float k,dk,fk;
    float complex cshift,*pp,w2;

    /* determine wavenumber sampling, pad by 2 */
    nk = nx*2;
    nk = sf_npfao(nk,nk*2);
    dk = 2.0*SF_PI/(nk*dx);
    fk = -SF_PI/dx;

    /* allocate workspace */
    pp  = sf_complexalloc (nk);
  
    if (!inv) { /* prepare image for migration */
	for (ix=0; ix<nx; ix++) {      
	    for (iz=0; iz<nz; iz++)
		q[ix][iz] = 0.0;
	}
    }

    /* loop over frequencies w */
    for (iw=0; iw<nw; iw++) {
	if (verb) sf_warning ("frequency %d of %d",iw+1,nw);

	w2 = dw*(eps+iw*I);
	w2 *= w2;

	if (inv) { /* modeling */
	    for (ix=0; ix<nx; ix++) {
		pp[ix] = q[ix][nz-1];
	    }
	    for (ix=nx; ix<nk; ix++) {
		pp[ix] = 0.;
	    }

	    /* loop over migrated times z */
	    for (iz=nz-2; iz>=0; iz--) {
		sf_pfacc(1,nk,pp);
		pp[nk/2+1] = 0.; /* oddball negative nyquist */

		for (ix=0; ix<nk; ix++) {
		    jx = (ix < nk/2)? ix + nk/2: ix - nk/2;
		    k = fk + jx*dk;
		    k *= k;
	  
		    cshift = cexpf((csqrtf(w2*v[iz])-
				    csqrtf(w2*v[iz]+k))*dz); 
		    pp[ix] *= cshift/nk; 
		    /* FFT scaling included */
		}
		
		sf_pfacc(-1,nk,pp);

		for (ix=0; ix<nx; ix++) {
		    cshift = cexpf(-csqrtf(w2*vt[ix][iz])*dz);
		    pp[ix]= q[ix][iz] + pp[ix]*cshift;
		}
		for (ix=nx; ix<nk; ix++) {
		    pp[ix] = 0.;
		}
	    }

	    for (ix=0; ix<nx; ix++) {
		cp[ix][iw] = pp[ix];
	    }
	} else { /* migration */
    
	    for (ix=0; ix<nx; ix++) {
		pp[ix] = cp[ix][iw];
	    }
	    for (ix=nx; ix<nk; ix++) {
		pp[ix] = 0.;
	    }

	    /* loop over migrated depths z */
	    for (iz=0; iz<nz; iz++) {
		/* accumulate image (summed over frequency) */
		for (ix=0; ix<nx; ix++) { 
		    if (iw==0)
			q[ix][iz] += (iw==0)? 
			    crealf(pp[ix]): 
			    2.*crealf(pp[ix]);
		}

		sf_pfacc(1,nk,pp);
		pp[nk/2+1] = 0.0; /* oddball negative nyquist */

		for (ix=0; ix<nk; ix++) {
		    jx = (ix < nk/2)? ix + nk/2: ix - nk/2;
		    k = fk + jx*dk;
		    k *= k;
	  
		    cshift = conjf(cexpf((csqrtf(w2*v[iz])-
					  csqrtf(w2*v[iz]+k))*dz));
		    pp[ix] *= cshift/nk;
		    /* Fourier scaling included */
		}

		sf_pfacc(-1,nk,pp);
	
		for (ix=0; ix<nx; ix++) {
		    cshift = conjf(cexpf(-csqrtf(w2*vt[ix][iz])*dz));
		    pp[ix] *= cshift;
		}
		for (ix=nx; ix<nk; ix++) {
		    pp[ix] = 0.;
		}
	    }
	}
    }
    
    free (pp);
}
