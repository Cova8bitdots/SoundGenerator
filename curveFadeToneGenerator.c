#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#define LENGTH 3
#define MAX_AMPLITUDE 32768
#define FREQ 440
#define SAMPLING_FREQ 44100
#define BIT_DEPTH 16
#define MONORAL 1

typedef struct 
{
    char *id;         //'R' 'I' 'F' 'F'
    int fileSize;       //
    char *wavId;      // 'W' 'A' 'V' 'E'
    char *chunkId;    // 'f' 'm' 't' ' '
    int chunkSize;      // 16
    short fmtId;        // 1
    short channel;      //1:mono 2:stereo
    int samplingRate;   // 44100/48000[Hz]
    int bytePerSec;     // Fs * BitDepth * channel
    short blockSize;    // BitDepth(byte) * channel
    short bitDepth;     // bit / sample
    char *dataId;     //'d' 'a' 't' 'a'
    int dataSize;       // 波形長[sample]
    double *wavData;    // データ
}PCM_DATA;

void GenerateSinCurveTone( PCM_DATA *data )
{
    int i;
    int fs = data->samplingRate;
    if( data->channel == MONORAL )
    {
        for( i = 0; i < data->dataSize; i++)
        {
            data->wavData[i] = ( sin( 2.0*M_PI*i *FREQ/fs) );
        }
    }
    else
    {
        for( i = 0; i < data->dataSize/2; i++)
        {
            data->wavData[2*i]      = ( sin( 2.0*M_PI*i *FREQ / fs ) );
            data->wavData[2*i+1]    = ( sin( 2.0*M_PI*i *FREQ / fs) );
        }
    }
    
}

// Wav Header 生成
void CreateWaveData( PCM_DATA *header, int length, short channel)
{
    // Tags
    header->id      ="RIFF";
    header->wavId   ="WAVE";
    header->chunkId = "fmt ";
    header->dataId  = "data";

    // params
    header->chunkSize    = 16;
    header->fmtId        = (short)1;
    header->channel      = channel;
    header->samplingRate = SAMPLING_FREQ;
    header->bitDepth     = BIT_DEPTH;
    header->bytePerSec   = SAMPLING_FREQ * BIT_DEPTH * channel;
    header->blockSize    = BIT_DEPTH / 8 * channel;

    // Size
    header->dataSize = length*SAMPLING_FREQ * channel;
    header->fileSize = 44 - 8 + header->dataSize;

    header->wavData = (double *)malloc( header->dataSize * sizeof(double) );
    // return header;
}

// 対象ファイルにHeader書き出し
void WriteWavData( PCM_DATA *data, FILE *fp)
{
    fwrite( data->id, 1, 4, fp);
    fwrite( &data->fileSize, 4, 1, fp);
    fwrite( data->wavId, 1, 4, fp);
    fwrite( data->chunkId, 1, 4, fp);
    fwrite( &data->chunkSize, 4, 1, fp);
    fwrite( &data->fmtId, 2, 1, fp);
    fwrite( &data->channel, 2, 1, fp);
    fwrite( &data->samplingRate, 4, 1, fp);
    fwrite( &data->bytePerSec, 4, 1, fp);
    fwrite( &data->blockSize, 2, 1, fp);
    fwrite( &data->bitDepth, 2, 1, fp);
    fwrite( data->dataId, 1, 4, fp);
    fwrite( &data->dataSize, 4, 1, fp);

    int i;
    short value;
    for( i = 0; i < data->dataSize; i++)
    {
        value = data->wavData[i] * MAX_AMPLITUDE;
        // Clamp
        if( value > MAX_AMPLITUDE-1)
        {
            value = MAX_AMPLITUDE -1;
        }
        else if( value < -MAX_AMPLITUDE )
        {
            value = -MAX_AMPLITUDE;
        }
        fwrite( &value, 2,1, fp);
    }
    
}


int main(int argc, char const *argv[])
{
    char fileName[128];
    FILE *fp;
    PCM_DATA data;
    
    printf("ファイル名を入力してください>");
    scanf("%s" , fileName);

    fp = fopen( fileName, "wb");
    if( fp == NULL )
    {
        fprintf(stderr, "このファイルをひらけませんでした(%s)\n", fileName);
        fclose( fp );
        return EXIT_FAILURE;
    }

    CreateWaveData( &data, LENGTH, MONORAL);
    GenerateSinCurveTone( &data );
    WriteWavData( &data, fp);
    free( data.wavData );
    fclose( fp );
    fprintf(stdout, "ファイルの生成が完了しました:(%s)\n", fileName);
    return EXIT_SUCCESS;
}
