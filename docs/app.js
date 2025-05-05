function initMap() {
    const map = new google.maps.Map(document.getElementById("map"), {
      center: { lat: 37.7749, lng: -122.4194 }, // Initial center (San Francisco)
      zoom: 13,
    });
  
    const input = document.getElementById("pac-input");
    const searchBox = new google.maps.places.SearchBox(input);
    map.controls[google.maps.ControlPosition.TOP_LEFT].push(input);
  
    let markers = [];
  
    searchBox.addListener("places_changed", () => {
      const places = searchBox.getPlaces();
  
      if (places.length === 0) return;
  
      // Clear old markers
      markers.forEach((marker) => marker.setMap(null));
      markers = [];
  
      const bounds = new google.maps.LatLngBounds();
  
      places.forEach((place) => {
        if (!place.geometry || !place.geometry.location) return;

        const marker = new google.maps.Marker({
          map,
          title: place.name,
          position: place.geometry.location,
        });
      
        marker.addListener("click", () => {
          const lat = place.geometry.location.lat();
          const lng = place.geometry.location.lng();
          document.getElementById("coordinates").value = `${lat.toFixed(6)}, ${lng.toFixed(6)}`;
        });


  
        bounds.extend(place.geometry.location);
      });
  
      map.fitBounds(bounds);
    });

    map.addListener("click", (e) => {
      const lat = e.latLng.lat();
      const lng = e.latLng.lng();
  
      // Set the coordinates into the input field
      document.getElementById("coordinates").value = `${lat.toFixed(6)}, ${lng.toFixed(6)}`;
    });
  };


  const addBtn = document.getElementById("addButton");
  const stopInput = document.getElementById("stopName");
  const routeList = document.getElementById("route-list");
  const coordInput = document.getElementById("coordinates");

  addBtn.addEventListener("click", () => {
    const liCount = routeList.querySelectorAll('li').length + 1;
    const stopName = stopInput.value.trim() || `Stop ${liCount}`;
    const coordinates = coordInput.value.trim();
  
    const li = document.createElement('li');
  
    // CREATE SPAN FOR LABEL
    const labelSpan = document.createElement('span');
    if (stopInput.value.trim() === '') {
      labelSpan.textContent = `${liCount}. Stop ${liCount}`;
    } else {
      labelSpan.textContent = `${liCount}. ${stopInput.value.trim()}`;
    }
    li.setAttribute('data-coordinates', coordinates);
    li.appendChild(labelSpan);
  
    // CREATE BUTTON CONTAINER
    const stopBtn = document.createElement('div');
    stopBtn.classList.add('stopBtn');
  
    // Edit button
    const editBtn = document.createElement('button');
    editBtn.textContent = 'Edit';
    editBtn.classList.add('edit-btn');
    stopBtn.appendChild(editBtn);
  
    // Delete button
    const deleteBtn = document.createElement('button');
    deleteBtn.textContent = 'Delete';
    deleteBtn.classList.add('delete-btn');
    stopBtn.appendChild(deleteBtn);
  
    li.appendChild(stopBtn);
    routeList.appendChild(li);
  
    stopInput.value = '';
    coordInput.value = '';
  
    deleteBtn.addEventListener("click", () => {
      routeList.removeChild(li);
      updateIndexes();
    });

    editBtn.addEventListener("click", () =>{
    const currentStopName = labelSpan.textContent.split('. ')[1];
    
    const input = document.createElement('input');
    input.type = 'text';
    input.value = currentStopName;
    input.classList.add('edit-input');
    
    // Replace the span with input
    li.replaceChild(input, labelSpan);

    // Change the Edit button to Save
    editBtn.textContent = 'Save';

    // When clicking Save, update the stop name
    editBtn.addEventListener('click', () => {
      const newStopName = input.value.trim() || `Stop ${liCount}`;
      labelSpan.textContent = `${liCount}. ${newStopName}`;
      
      // Replace the input with the updated span
      li.replaceChild(labelSpan, input);

      // Change Save button back to Edit
      editBtn.textContent = 'Edit';
      updateIndexes();
    });
    })
  });

  function updateIndexes() {
    const items = routeList.querySelectorAll('li');
    items.forEach((item, index) => {
      const labelSpan = item.querySelector('span');
      const text = labelSpan.textContent.trim();
  
      if (text.match(/^\d+\.\s+Stop \d+$/)) {
        // Matches pattern like "1. Stop 1" → update both numbers
        labelSpan.textContent = `${index + 1}. Stop ${index + 1}`;
      } else {
        // Custom name → update only the prefix number
        const nameParts = text.split('. ');
        const nameWithoutNumber = nameParts.slice(1).join('. ') || nameParts[0];
        labelSpan.textContent = `${index + 1}. ${nameWithoutNumber}`;
      }
    });
  }
  
  document.getElementById("saveButton").addEventListener("click", () => {
    const stops = document.querySelectorAll('#route-list li');
    const activityType = document.getElementById("select-sport").value;

    // Add a header line with the activity type once
    let csvContent = `${activityType}\n`;

    stops.forEach((stop) => {
      const fullText = stop.querySelector('span').textContent.trim();
      const nameParts = fullText.split('. '); // splits ["1", "StopName"]
      const stopName = nameParts.slice(1).join('. '); // joins back "StopName" (handles cases where name has dots)
      
        const coordinates = stop.getAttribute('data-coordinates');
        const [lat, lng] = coordinates.split(',').map(str => str.trim());

        csvContent += `${stopName}\n`;
        csvContent += `${lat}\n`;
        csvContent += `${lng}\n`;
    });

    const blob = new Blob([csvContent], { type: 'text/plain;charset=utf-8;' });

    const link = document.createElement('a');
    const url = URL.createObjectURL(blob);
    link.setAttribute('href', url);
    link.setAttribute('download', 'stops.txt');
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
});

/*

  // Install the Web Serial API (Browser Support):
  // Make sure you’re using a Chrome or Edge browser that supports the Web Serial API. 
  // Safari and Firefox do not currently support the Web Serial API.
  
  // Function to connect to the Arduino via Serial
async function connectToArduino() {
  try {
    const port = await navigator.serial.requestPort();  // Request the serial port
    await port.open({ baudRate: 9600 }); // Open the serial port with a baud rate
    
    const writer = port.writable.getWriter();  // Get writer to send data
    
    // Convert CSV content to a Uint8Array for serial communication
    const encoder = new TextEncoder();
    const csvContent = "Stop Name, Coordinates, Activity Type\nStop 1, 40.7128, -74.0060, Walking\n";
    const data = encoder.encode(csvContent);  // Encode the CSV as bytes
    
    await writer.write(data);  // Send the data over serial
    
    writer.releaseLock();  // Release the writer lock
    
    // Close the port when done
    await port.close();
  } catch (error) {
    console.error('Error with serial communication:', error);
  }
}

// Connect to Arduino and send CSV when button is clicked
document.getElementById("saveButton").addEventListener("click", () => {
  connectToArduino();  // Send data to Arduino
});


*/

  
  


  

