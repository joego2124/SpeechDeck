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

  if(Firebase.auth().currentUser == undefined) {
    ui.start('#firebaseui-auth-container', uiConfig);
  }

  const [imageFile, setImageFile] = useState();
  const [audioFile, setAudioFile] = useState();
  const [name, setName] = useState('Macro-Name');
  const [message, setMessage] = useState();
  const [macroList, setMacroList] = useState(['default']);
  var layout = [];

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
    console.log(imageFile);
    Firebase.storage().ref(`/${uid}/${name}/image.bmp`).put(imageFile.binary, {contentType: imageFile.contentType}); 
    Firebase.storage().ref(`/${uid}/${name}/audio.wav`).put(audioFile.binary, {contentType: audioFile.contentType}); 
    setMessage(`Uploaded macro ${name}`);
    console.log('files uploaded!');
  }

  const uploadLayout = () => {
    const uid = Firebase.auth().currentUser.uid;
    console.log(layout);
    Firebase.database().ref(`/${uid}/Layout`).set(layout);
  }

  const changeName = (event) => {
    setName(event.target.value);
  }


  return (
    <div className="App">

      <div id="firebaseui-auth-container"></div>
      <div id="loader">Signed in as {Firebase.auth().currentUser?.displayName}</div>

      <div className='BigSpacer'/>

      <div className='Border'>
        <Button className='BigButton' onClick={addData}>Upload Macro</Button>
        <div className='WideBorder'>
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

        <div className='WideBorder'>
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
      </div>

      
      <div className='BigSpacer'/>

      <div className='Border'>
        <Button className='BigButton' onClick={uploadLayout}>Upload Layout</Button>
        <div className='Row'>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 1" onSelect={ macro => {layout[0] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 2" onSelect={ macro => {layout[1] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 3" onSelect={ macro => {layout[2] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 4" onSelect={ macro => {layout[3] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 5" onSelect={ macro => {layout[4] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        </div>
        <div className='Row'>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 6" onSelect={ macro => {layout[5] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 7" onSelect={ macro => {layout[6] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 8" onSelect={ macro => {layout[7] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 9" onSelect={ macro => {layout[8] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 10" onSelect={ macro => {layout[9] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        </div>
        <div className='Row'>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 11" onSelect={ macro => {layout[10] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 12" onSelect={ macro => {layout[11] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 13" onSelect={ macro => {layout[12] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 14" onSelect={ macro => {layout[13] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        <DropdownButton className='PaddedButton' id="dropdown-basic-button" title="Select Macro 15" onSelect={ macro => {layout[14] = macro;} }>
          {
            macroList.map( macro => {
              return (
                <Dropdown.Item key={macro} eventKey={macro}>{macro}<br/></Dropdown.Item>
              );
            })
          }
        </DropdownButton>
        </div>
      </div>
    </div>
  );
}

export default App;
