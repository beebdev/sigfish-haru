#!/bin/bash

set -e

HARU_VENV=~/haru/
MINIMAP2=minimap2
FASTQ=test/sp1/batch0.fastq
REF=test/sp1/nCoV-2019.reference.fasta
BLOW5=test/sp1/batch0.blow5
THREADS=8
REF_PAF=test/sp1/batch0.minimap2.paf
MY_PAF=test/test.paf

make
#${MINIMAP2} -cx map-ont ${REF} ${FASTQ} --secondary=no -t ${THREADS} > ${REF_PAF}
#./sigfish dtw -g ${REF} -s ${BLOW5} -t ${THREADS} --from-end > ${MY_PAF}
./sigfish dtw -g ${REF} -s ${BLOW5} -t ${THREADS}  > ${MY_PAF}

source ${HARU_VENV}/bin/activate
uncalled pafstats -r ${REF_PAF} ${MY_PAF}
