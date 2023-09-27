  #include <stdio.h>
  #include <string.h>
  #include <math.h>
  #include <stdlib.h>
  #include <ctype.h>
  #include <getopt.h>
  #include <stdbool.h>
  #include <stdint.h>

  //this demo is finished under discussions with Guochen Liao and Yin Suo
  const double pi = 3.14;

  typedef struct header{
    char maker [4]; //RIFF
    uint32_t chunk_size;
    char format [4]; //WAVE
    char first_subchunkID [4]; //"fmt "
    uint32_t first_subchunk_size;
    uint16_t audio_format; //1
    uint16_t number_of_channels; //1
    uint32_t sample_rate; //44100
    uint32_t byte_rate; //sample_rate * number_of_channels * bytes_per_sample
    uint16_t block_alignment; //number_of_channels * bytes_per_sample
    uint16_t bits_per_sample; //32
    char second_subchunkID [4];//data
    uint32_t second_subchunk_size;
  } header;

  header* make_header() {
    header *h = (header*) malloc(sizeof(header));
    memcpy(h->maker, "RIFF", 4);
    h->chunk_size = 0;
    memcpy(h->format, "WAVE", 4);
    memcpy(h->first_subchunkID, "fmt ", 4);
    h->first_subchunk_size = 16;
    h->audio_format = 1;
    h->number_of_channels = 1;
    h->sample_rate = 44100;
    h->byte_rate = 44100 * 1 * 4;
    h->block_alignment = 1 * 4;
    h->bits_per_sample = 32;
    memcpy(h->second_subchunkID, "data", 4);
    h->second_subchunk_size = 0;
    return h; 
  }

  void write_header(FILE *file, int32_t sz) {
      header *h = make_header();
      h->second_subchunk_size = sz;
      h->chunk_size = sizeof(header) + h->second_subchunk_size - 8;
      fwrite(h, sizeof(header), 1, file);
      free(h);  // Free the memory allocated for the header.
  }

  // Returns the number of samples contained a wave file (with a sample rate of
  // 44.1 kHz and 1 channel) that plays for dur seconds.
  int num_samples_from_duration(int dur){
    return 44100 * 1 * dur;
  }

  // Returns a pointer to a heap-allocated buffer containing the sound data for
  // a simple triangle wave that lasts for dur seconds.
  uint32_t *make_simple_sawtooth_data(int dur){
    uint32_t *temp = (uint32_t*) malloc (4 * num_samples_from_duration(dur));
    for(uint32_t i = 0; i < num_samples_from_duration(dur); i++){
      if(i == 0){
        temp[i] = 0;
      }else{
        temp[i] = temp[i - 1] + 8000000;
      }
    }
    return temp;
  }

  // Writes a complete wave file to a file named filename using the sound data
  // pointed to by data of which contains len samples.
  void write_wave(const char *filename, uint32_t len, uint32_t *data){
    FILE *fp = fopen(filename, "wb");
    write_header(fp, sizeof(uint32_t) * len);
    fwrite(data, sizeof(uint32_t), len, fp);
    fclose(fp);
  }

  // Given a float f in the range [-1.0, 1.0], return a sample in the range
  // [0, INT32_MAX].
  uint32_t convert_to_sample(float f){
    if(f != 0){
      return INT32_MAX * (f + 1) / 2;
    }else{
      return INT32_MAX / 2;
    }
  }

  // Returns a pointer to a heap-allocated buffer containing the sound data for
  // a sine wave of frequency freq that lasts for dur seconds.
  uint32_t* make_sine_data(int dur, int freq){
      uint32_t *temp = (uint32_t*) malloc (4 * num_samples_from_duration(dur));
      for(uint32_t i = 0; i < num_samples_from_duration(dur); i++){
          float sine_val = sin(2 * pi * i * freq / 44100);
          temp[i] = convert_to_sample(sine_val);
      }
      return temp;
  }

  // Returns a pointer to a heap-allocated buffer containing the sound data for
  // a square wave of frequency freq that lasts for dur seconds.
  uint32_t* make_square_data(int dur, int freq){
    int count = 0;
    uint32_t *temp = (uint32_t*) malloc (4 * num_samples_from_duration(dur));
    for(uint32_t i = 0; i < dur * freq; i++){
      for(uint32_t j = 0; j < num_samples_from_duration(1) / freq / 2; j++){
        temp[count] = UINT32_MAX + 1;
        count++;
      }
      for(uint32_t j = 0; j < num_samples_from_duration(1) / freq / 2; j++){
        temp[count] = UINT32_MAX;
        count++;
      }
    }
    return temp;
  }

  // Returns a pointer to a heap-allocated buffer containing the sound data for
  // a saw wave of frequency freq that lasts for dur seconds.
  uint32_t* make_saw_data(int dur, int freq){
    uint32_t *temp = (uint32_t*) malloc (4 * num_samples_from_duration(dur));
    for(uint32_t i = 0; i < dur; i++){
      for(uint32_t j = i * num_samples_from_duration(1); j < (i + 1) * num_samples_from_duration(1); j++){
        uint32_t index = (j - i * num_samples_from_duration(1)) / num_samples_from_duration(1);
        temp[j] = convert_to_sample(2 * index * freq - floor(1 / 2 + index * freq));
      }
    }
    return temp;
  }

  // Returns a pointer to a heap-allocated buffer containing the sound data for
  // a triangle wave of frequency freq that lasts for dur seconds.
  uint32_t* make_triangle_data(int dur, int freq){
    uint32_t *temp = (uint32_t*) malloc (4 * num_samples_from_duration(dur));
    for(uint32_t i = 0; i < dur; i++){
      for(uint32_t j = i * num_samples_from_duration(1); j < (i + 1) * num_samples_from_duration(1); j++){
        temp[j] = convert_to_sample(4 * freq * ((((j - i * num_samples_from_duration(1)) / num_samples_from_duration(1) - 1 / 4 * freq) % (1 / freq))+ (1 / freq)) % (1 / freq) - 1 / 2 * freq);
      }
    }
    return temp;
  }

  // Modifies the sound data found in data of length len to have n pulses of sound
  // using a simple linearly decaying envelope.
  void apply_pulse_envelope(int len, uint32_t *data, int n){
    for(uint32_t i = 0; i < n; i++){
      for(uint32_t j = i * len / n; j < (i + 1) * len / n; j++){
        data[j] = (1 - (j - i * len / n) / (len / n)) * UINT32_MAX;
      }
    }
  }

    // 检查文件名是否以 ".wav" 结束
  bool has_wav_suffix(const char *filename) {
    size_t len = strlen(filename);
    return len > 4 && strcmp(filename + len - 4, ".wav") == 0;
  }

  // 如果需要，为文件名添加 ".wav" 后缀
  char* ensure_wav_suffix(const char *filename) {
    if (has_wav_suffix(filename)) {
      return strdup(filename);
    } else {
      char *new_name = (char*) malloc(strlen(filename) + 5); // 5 = 4 for ".wav" + 1 for '\0'
        sprintf(new_name, "%s.wav", filename);
        return new_name;
    }
  }


  int main(int argc, char *argv[]) {
    int opt;
    char* form = "sine";
    float freq = 44100.0;
    int duration = 10;
    int pulse = 1;
    while ((opt = getopt(argc, argv, "w:f:d:p:")) != -1) {
      switch(opt) {
        case 'w' :
          form = optarg;
          break; 
        case 'f' :
          freq = atof(optarg);
          break;
        case 'd' : 
          duration = atoi(optarg);
          break;
        case 'p' :
          pulse = atoi(optarg);
          break;
        }
    }

  char* new_filename = "test.wav";
  uint32_t *temp = make_simple_sawtooth_data(5);
  write_wave(new_filename, num_samples_from_duration(5), temp);
  free(temp);

  char *filename = NULL;
  if (optind < argc) {
      char *original_name = argv[optind];
      filename = ensure_wav_suffix(original_name); // 使用 ensure_wav_suffix 函数
  } else {
      // 如果没有提供文件名，可以使用默认文件名，如 "output.wav"
      filename = strdup("output.wav");
  }

  uint32_t* temp_new = NULL;
  if (strcmp(form, "sine") == 0) {
      temp_new = make_sine_data(duration, freq);
  } else if (strcmp(form, "square") == 0) {
      temp_new = make_square_data(duration, freq);
  } else if (strcmp(form, "saw") == 0) {
      temp_new = make_saw_data(duration, freq); 
  } else if (strcmp(form, "triangle") == 0) {
      temp_new = make_triangle_data(duration, freq);
  }
  write_wave(filename, num_samples_from_duration(10), temp_new);
  free(temp_new);
  free(filename);
  return 0;
  }