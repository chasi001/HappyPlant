
function water() {
  alert('watering started');
}

function auto(value) { // function to not accept empty or negative values
  var limit  = Number(value);
  if(limit < 1 || limit >= 100) {
    alert("Moisture limit cannot be empty or negative and max is 100%!");
  }

}


