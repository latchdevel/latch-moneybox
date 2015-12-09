/*
 * MoneyBox 11Paths (c)Nov2015
 * 
 * Copyright (C) 2013 Eleven Paths
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#include <aJSON.h>
#include "mb_json.h"

aJsonObject* read_json_file(char* file_name){
  char c;
  long i=0; 
  char *file_buffer;
  FILE *file;
  aJsonObject *json_file = NULL;
  
  file_buffer = (char*)malloc(sizeof(char)*FILE_BUFFER_SIZE);
  if (file_buffer!=NULL){
    memset(file_buffer, 0, sizeof(char)*FILE_BUFFER_SIZE);
    file = fopen(file_name, "r");
    if (file) {
      while (((( c = fgetc(file) ) != EOF ) and (i<FILE_BUFFER_SIZE))){
        file_buffer[i]=c;
        i++;
      }//end while
      fclose(file);
      file_buffer[i]='\0';
      if (i<FILE_BUFFER_SIZE){
          json_file = aJson.parse(file_buffer);
          if (json_file!=NULL){
              free(file_buffer);
              return json_file;
          }else{
            // fail parse json
            //SerialV1.println("DEBUG: json parse eoror");
            free(file_buffer);
            return NULL;  
          }   
      }else{
        // fail buffer overflow
        //SerialV1.println("DEBUG: json file buffer overflow");
        free(file_buffer);
        return NULL;
        } 
    }else{
      // fail file open
      //SerialV1.println("DEBUG: file open error");
      free(file_buffer);
      return NULL;
      }
  }else{
    // fail malloc
    //SerialV1.println("DEBUG: malloc error");
    return NULL;
  }
  // anything else
  return NULL; 
}

int id_index(aJsonObject* jsonObject,int id){
  
  int r=-1;
  int i=0;
  int n=0;
  aJsonObject *json_temp = NULL;
  
  if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
      jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
      if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
            i=aJson.getArraySize(jsonObject);
            if (i>0){
              while ((n<i) and (r<0)){              
                  json_temp=aJson.getArrayItem(jsonObject, n);
                  if (json_temp!=NULL){
                      if (json_temp->type==aJson_Object){
                          json_temp = aJson.getObjectItem(json_temp,JSON_ID);
                          if (json_temp->type==aJson_Int){
                            if (json_temp->valueint==id){
                              r=n;
                            }
                          }                          
                      }
                  }
                n++;
              }  
            }else{
              // fail emply array
              r=-2; 
            }  
          }else{
            // fail moneybox object is not an array
            r=-3;
          }  
      }else{
        // fail moneybox object is not an array
        r=-4;
      }
    }else{
      //fail jsonObject not an Objetc
      r=-5;    
    }
  }else{
    //fail jsonObject null
    r=-6;  
  }
  return r;  
} 

int num_latchids(aJsonObject* jsonObject,int index){
  
  int i=-1;
  aJsonObject *json_temp = NULL;
  
  if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
      jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
      if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
             json_temp=aJson.getArrayItem(jsonObject, index);
             if (json_temp!=NULL){
                if (json_temp->type==aJson_Object){
                    json_temp = aJson.getObjectItem(json_temp,JSON_LATCH_ACCOUNTS_ARRAY);
                    if (json_temp->type==aJson_Array){
                      i=aJson.getArraySize(json_temp);
                    }else{
                      // fail latch account not an array
                      i=-2;  
                    }                        
                 }else{
                    // fail el elemento (id) no es un objeto 
                    i= -3;
                 }
             }else{
                //  fail el elemento x no está en el array de ids
                i= -4;
             }           
          }else{
            // fail moneybox object is not an array
            i= -5;
          }  
      }else{
        // fail moneybox object is not an array
        i= -6;
      }
    }else{
      //fail jsonObject not an Objetc
      i= -7;    
    }
  }else{
    //fail jsonObject null
    i= -8;  
  }
  // Anything else
  return i;  
}

char* get_latchid(aJsonObject* jsonObject,int index_id, int index_latch){
  
  char* r = NULL;
  aJsonObject *json_temp = NULL;
  
  if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
      jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
      if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
             json_temp=aJson.getArrayItem(jsonObject, index_id);
             if (json_temp!=NULL){
                if (json_temp->type==aJson_Object){
                    json_temp = aJson.getObjectItem(json_temp,JSON_LATCH_ACCOUNTS_ARRAY);
                    if (json_temp->type==aJson_Array){
                    json_temp=aJson.getArrayItem(json_temp, index_latch);
                    if (json_temp!=NULL){
                      if (json_temp->type==aJson_Object){
                          json_temp = aJson.getObjectItem(json_temp,JSON_LATCH_ACCOUNT);
                          if (json_temp->type==aJson_String){
                            r=json_temp->valuestring;
                          }
                       }                          
                     }
                    }else{
                      // fail latch account not an array
                    }                        
                 }else{
                    // fail el elemento (id) no es un objeto 
                 }
             }else{
                //  fail el elemento x no está en el array de ids
             }           
          }else{
            // fail moneybox object is not an array
          }  
      }else{
        // fail moneybox object is not an array
      }
    }else{
      //fail jsonObject not an Objetc   
    }
  }else{
    //fail jsonObject null
  }
  return r;  
}

int num_ids(aJsonObject* jsonObject){
    int i=0;
    if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
        jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
        if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
            i=aJson.getArraySize(jsonObject);
          }else{
            // fail moneybox object is not an array
            i=-1;  
          }
        }else{
          // fail getObjetItem "moneybox"
          i=-2;
        }
    }else{
      // jsonObject not an Object
      i=-3; 
    }
  }else{
      // jsonObject NULL
      i=-4;
  }
  return i;  
}

boolean latch_and(aJsonObject* jsonObject,int index){
  boolean r=false;
  aJsonObject *json_temp = NULL;
  
  if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
      jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
      if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
             json_temp=aJson.getArrayItem(jsonObject, index);
             if (json_temp!=NULL){
                if (json_temp->type==aJson_Object){
                    json_temp = aJson.getObjectItem(json_temp,JSON_LATCH_OPER);
                          if (json_temp->type==aJson_String){
                            if (strstr(json_temp->valuestring, "AND") != NULL){
                              r= true;
                          }else{
                            // fail operation no es AND
                          }  
                       }else{
                        // fail operation no es un string 
                       }                                               
                 }else{
                    // fail el elemento (id) no es un objeto 
                 }
             }else{
                //  fail el elemento x no está en el array de ids
             }           
          }else{
            // fail moneybox object is not an array
          }  
      }else{
        // fail moneybox object is not an array
      }
    }else{
      //fail jsonObject not an Objetc  
    }
  }else{
    //fail jsonObject null
  }
  // Anything else
  return r;  
}

char* get_money(aJsonObject* jsonObject,int index_id){
  char* r = NULL;
  aJsonObject *json_temp = NULL;
  
  if (jsonObject!=NULL){
      if (jsonObject->type==aJson_Object){
      jsonObject = aJson.getObjectItem(jsonObject,JSON_ROOT);
      if (jsonObject!=NULL){
          if (jsonObject->type==aJson_Array){
             json_temp=aJson.getArrayItem(jsonObject, index_id);
             if (json_temp!=NULL){
                if (json_temp->type==aJson_Object){
                          json_temp = aJson.getObjectItem(json_temp,JSON_LATCH_MONEY);
                          if (json_temp->type==aJson_String){
                            r=json_temp->valuestring;
                          }                                    
                 }else{
                    // fail el elemento (id) no es un objeto 
                 }
             }else{
                //  fail el elemento x no está en el array de ids
             }           
          }else{
            // fail moneybox object is not an array
          }  
      }else{
        // fail moneybox object is not an array
      }
    }else{
      //fail jsonObject not an Objetc   
    }
  }else{
    //fail jsonObject null
  }
  return r;  
}


// No usada
boolean latch_state(aJsonObject* jsonResponse,char* operation_id){
   boolean r=false;
   if ((jsonResponse!=NULL)and(jsonResponse->type==aJson_Object)){
      jsonResponse=aJson.getObjectItem(jsonResponse,"data");
      if ((jsonResponse!=NULL)and(jsonResponse->type==aJson_Object)){
          jsonResponse=aJson.getObjectItem(jsonResponse,"operations");
          if ((jsonResponse!=NULL)and(jsonResponse->type==aJson_Object)){
             jsonResponse=aJson.getObjectItem(jsonResponse,operation_id);
             if ((jsonResponse!=NULL)and(jsonResponse->type==aJson_Object)){
              jsonResponse=aJson.getObjectItem(jsonResponse,"status");
              if ((jsonResponse!=NULL)and(jsonResponse->type==aJson_String)){
                  if(jsonResponse->valuestring){
                    r=true;  
                  }           
               }
             }
          }
      }
   }
  return r;
}
