import './App.css';
import FileUpload from './FileUpload';
import React, { useState, useEffect } from 'react';
import Firebase from "firebase";
import config from './config';
import uiConfig from './uiConfig';
import { Button, Dropdown, DropdownButton } from 'react-bootstrap';

var firebaseui = require('firebaseui');

function App() {
	if (!Firebase.apps.length) {
		Firebase.initializeApp(config);
	}else {
		Firebase.app(); // if already initialized, use that one
	}

  // Initialize the FirebaseUI Widget using Firebase.
  var ui = firebaseui.auth.AuthUI.getInstance() ?? new firebaseui.auth.AuthUI(Firebase.auth());

  if(ui.isPendingRedirect()) {
    ui.start('#firebaseui-auth-container', uiConfig);
  }


  const [imageFile, setImageFile] = useState();
  const [audioFile, setAudioFile] = useState();
  const [name, setName] = useState('Macro-Name');
  const [message, setMessage] = useState();
  const [macroList, setMacroList] = useState(['default']);

  useEffect(() => setTimeout( () => listMacros(), 500), []);

  const listMacros = () => {
    const uid = Firebase.auth().currentUser?.uid;
    if (uid !== undefined)
      Firebase.storage().ref(`/${uid}`).listAll().then(snapshot => {
        var tempList = [];
        snapshot.prefixes.forEach(prefix => {
          tempList.push(prefix.name);
        });
        setMacroList(tempList);
        console.log('macros:',tempList);
      });
    else
      console.log(uid, Firebase.auth().currentUser);
  }

  const addData = () => {

    if(name.length <= 0) {
      setMessage('Please set a macro name');
      return;
    }
    if(imageFile === undefined){
      setMessage('Please select an image file')
      return;
    }
    if(audioFile === undefined){
      setMessage('Please select an audio file')
      return;
    }

    listMacros()

    const uid = Firebase.auth().currentUser.uid;
    Firebase.storage().ref(`/${uid}/${name}/image`).put(imageFile.data, {contentType: imageFile.contentType}); 
    Firebase.storage().ref(`/${uid}/${name}/audio`).put(audioFile.data, {contentType: audioFile.contentType}); 
    setMessage(`Uploaded macro ${name}`);
    console.log('files uploaded!');
  }

  const changeName = (event) => {
    setName(event.target.value);
  }


  return (
    <div className="App">

      <div id="firebaseui-auth-container"></div>
      <div id="loader">Signed in as {Firebase.auth().currentUser?.displayName}</div>
      
      <div className='Spacer'/>
      <div className='Spacer'/>

      <div className='Border'>
        <div className='Spacer'/>
        <DropdownButton id="dropdown-basic-button" title="Select Existing Macro" onSelect={ macro => setName(macro) }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <div className='Spacer'/>
          
        <label htmlFor="macroInput">Macro Name: </label>
        <input id='MacroInput' type='text' value={name} onChange={changeName} />
        <div className='Spacer'/>
      </div>

      <div className='Spacer'/>

      <div className='Border'>
        <div className='Row'>
          <div className='Column'>
            <FileUpload setter={setAudioFile} message = 'Click to Select Audio'/>
            <p> {audioFile?.fileName ?? 'No File'} Selected </p>
          </div>
          <div className='Column'>
            <FileUpload setter={setImageFile} message = 'Click to Select Image'/>
            <p> {imageFile?.fileName ?? 'No File'} Selected </p>
          </div>
        </div>
      </div>
      <p>{message}</p>
      <Button onClick={addData}>Upload Files</Button>
    </div>
  );
}

export default App;
