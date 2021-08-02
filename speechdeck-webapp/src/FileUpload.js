import React, { useCallback } from 'react';
import { useDropzone } from 'react-dropzone';
import { Button } from 'react-bootstrap';
// import Firebase from "firebase";

// import './styles.css';

const PLCFileUpload = ({ setter, message }) => {
  const onDrop = useCallback((acceptedFiles) => {
    acceptedFiles.forEach((file) => {
      const reader = new FileReader();

      reader.onabort = () => console.log('file reading was aborted')
      reader.onerror = () => console.log('file reading has failed')
      reader.onload = () => {
      // Do whatever you want with the file contents
        const binaryStr = reader.result;
        console.log(file, binaryStr);
        setter({binary: binaryStr, fileName: file.name, contentType: file.type});
      }
      reader.readAsArrayBuffer(file);
    })
    
  }, [setter])
  const { getRootProps, getInputProps } = useDropzone({ onDrop });

  return (
    <div {...getRootProps()}>
      <input {...getInputProps()} />
      <Button variant='light' className='uploadPLCButton'>
        <div className='buttonDiv'>
          <div className='buttonText'>{message}</div>
        </div>
      </Button>
    </div>
  );
};

export default PLCFileUpload;
