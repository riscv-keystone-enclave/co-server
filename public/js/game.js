var firebaseConfig = {
  apiKey: "AIzaSyAkU7OmFS2V1CXLD8fOYr-0rutookFTeak",
  authDomain: "npcasc2020.firebaseapp.com",
  databaseURL: "https://npcasc2020.firebaseio.com",
  projectId: "npcasc2020",
  storageBucket: "npcasc2020.appspot.com",
  messagingSenderId: "103760217336",
  appId: "1:103760217336:web:92b17e1d4e11e39635670d",
  measurementId: "G-NDY3JGGC8H"
};
var version = 111;

firebase.initializeApp(firebaseConfig);
var database = firebase.database().ref().child('scores');
var startTime, interval;
var time_minutes = document.getElementById("minutes");
var time_second = document.getElementById("seconds");
var time_miliseconds = document.getElementById("miliseconds");
var current_num = document.getElementById("demo");
var start_game_btn = document.getElementById("str_btn");
var sub_score_btn = document.getElementById("sub_score_btn");
var close_gm_btn = document.getElementById("close_gm_btn");
var btn_11 = document.getElementById("bt11");
var btn_12 = document.getElementById("bt12");
var btn_13 = document.getElementById("bt13");
var btn_14 = document.getElementById("bt14");
//var btn_15 = document.getElementById("bt15");
var btn_21 = document.getElementById("bt21");
var btn_22 = document.getElementById("bt22");
var btn_23 = document.getElementById("bt23");
var btn_24 = document.getElementById("bt24");
//var btn_25 = document.getElementById("bt25");
var btn_31 = document.getElementById("bt31");
var btn_32 = document.getElementById("bt32");
var btn_33 = document.getElementById("bt33");
var btn_34 = document.getElementById("bt34");
//var btn_35 = document.getElementById("bt35");
var btn_41 = document.getElementById("bt41");
var btn_42 = document.getElementById("bt42");
var btn_43 = document.getElementById("bt43");
var btn_44 = document.getElementById("bt44");
//var btn_45 = document.getElementById("bt45");
// var btn_51 = document.getElementById("bt51");
// var btn_52 = document.getElementById("bt52");
// var btn_53 = document.getElementById("bt53");
// var btn_54 = document.getElementById("bt54");
// var btn_55 = document.getElementById("bt55");

for (var arr_seq=[],i=0;i<32;++i) arr_seq[i]=i + 1;
var arr_first=[];
var arr_two=[];

function shuffle(array) {
  var tmp, current, top = array.length;
  if(top) while(--top) {
    current = Math.floor(Math.random() * (top + 1));
    tmp = array[current];
    array[current] = array[top];
    array[top] = tmp;
  }
  return array;
}

function assign_val_btn(btn_id, ass_value){
	var btn = document.getElementById(btn_id.toString());
	if(ass_value > 32){
		btn.innerHTML = "";
		btn.value = 0;
	}
	else{
		btn.innerHTML = ass_value;
		btn.value = ass_value;
	}	
}

function assign_fisrt_round(){
	current_num.innerHTML = 1;
	for (arr_first=[],i=0;i<16;++i) arr_first[i]=i + 1;
	for (arr_two=[],i=0;i<16;++i) arr_two[i]=i + 17;
	for (arr_seq=[],i=0;i<32;++i) arr_seq[i]=i + 1;


	arr_first = shuffle(arr_first);
	arr_two = shuffle(arr_two);
	arr_first = shuffle(arr_first);
	arr_two = shuffle(arr_two);
	for(var i = 0; i < 4; i++){
		for(var j = 0; j < 4; j++){
			assign_val_btn("bt" + ((i+1) * 10 + (j+1)).toString(), arr_first[4 * i + j]);
		}
	}
}

function assign_zero_round(){
	stop();
	current_num.innerHTML = 1;
	for(var i = 0; i < 4; i++){
		for(var j = 0; j < 4; j++){
			assign_val_btn("bt" + ((i+1) * 10 + (j+1)).toString(), 0);
		}
	}
}

function assign_second_round(btn_id){
	if(arr_two.length > 0){
		assign_val_btn(btn_id, arr_two[0]);
		arr_two.shift();
	}
	else{
		assign_val_btn(btn_id, 40);
	}
}

function check_click_btn(event){
	// console.log(event.target.id);
	// console.log(event.srcElement.value);
	if(event.srcElement.value == arr_seq[0]){
		arr_seq.shift();
		assign_second_round(event.target.id);
		current_num.innerHTML = arr_seq[0];
	}
	if(arr_seq.length <= 0){
		assign_second_round(event.target.id);	
		clearInterval(interval);
		current_num.innerHTML = 32;	
    // uploadfirebase();
	}
}

function start(){
    startTime = Date.now();
    interval = setInterval(function(){
        updateDisplay(Date.now() - startTime);
    });
}

function stop(){
    clearInterval(interval);
    time_minutes.value = 0;
    time_second.value = 0;
    time_miliseconds.value = 0;
}

function updateDisplay(currentTime){
    // do your stuff
    var min = Math.floor(currentTime / 60000);
    var sec = Math.floor(currentTime / 1000);
    if(min < 1 && sec < 1){
    	time_minutes.value = 0;
    	time_second.value = 0;
    	time_miliseconds.value = currentTime;
    }
    if(sec >= 1 && min < 1){
    	time_minutes.value = 0;
    	time_second.value = Math.floor((currentTime - 60000 * min) / 1000);
    	time_miliseconds.value = currentTime - 1000 * sec;
    }
    if(min >= 1){
    	time_minutes.value = min;
    	time_second.value = Math.floor((currentTime - 60000 * min) / 1000);
    	time_miliseconds.value = currentTime - sec * 1000;
    }
    if(min >= 5){
    	stop();
    }

}

function uploadfirebase(){
  var playtime = time_minutes.value + "min" + time_second.value + "sec" + time_miliseconds.value;
  console.log(time_minutes.value.toString() + time_second.value.toString() + time_miliseconds.value.toString());
  if(arr_seq.length <= 0){
    var username = prompt("Your time is : " + playtime + "\n Enter player name below");
    if(username.length <= 0){
      alert("You must enter name");
    }
    else{
      var key = database.push().getKey();
      database.child(key).set({
        username: username,
        time: ((time_minutes.value - 0) * 60000 + (time_second.value-0) * 1000 + (time_miliseconds.value - 0)).toString()
      });
      assign_zero_round();
        for (arr_seq=[],i=0;i<32;++i) arr_seq[i]=i + 1;
      stop();
    }
  }
  else{
    alert("Finish the game and upload score!!");
  }
}

start_game_btn.addEventListener("click", assign_zero_round);
start_game_btn.addEventListener("click", assign_fisrt_round);
start_game_btn.addEventListener("click", start);
sub_score_btn.addEventListener("click", uploadfirebase);
btn_11.addEventListener("click", check_click_btn);
btn_12.addEventListener("click", check_click_btn);
btn_13.addEventListener("click", check_click_btn);
btn_14.addEventListener("click", check_click_btn);
// btn_15.addEventListener("click", check_click_btn);
btn_21.addEventListener("click", check_click_btn);
btn_22.addEventListener("click", check_click_btn);
btn_23.addEventListener("click", check_click_btn);
btn_24.addEventListener("click", check_click_btn);
// btn_25.addEventListener("click", check_click_btn);
btn_31.addEventListener("click", check_click_btn);
btn_32.addEventListener("click", check_click_btn);
btn_33.addEventListener("click", check_click_btn);
btn_34.addEventListener("click", check_click_btn);
// btn_35.addEventListener("click", check_click_btn);
btn_41.addEventListener("click", check_click_btn);
btn_42.addEventListener("click", check_click_btn);
btn_43.addEventListener("click", check_click_btn);
btn_44.addEventListener("click", check_click_btn);
// btn_45.addEventListener("click", check_click_btn);
// btn_51.addEventListener("click", check_click_btn);
// btn_52.addEventListener("click", check_click_btn);
// btn_53.addEventListener("click", check_click_btn);
// btn_54.addEventListener("click", check_click_btn);
// btn_55.addEventListener("click", check_click_btn);
$(document).on('shown.bs.modal', stop);
$(document).on('shown.bs.modal', assign_zero_round);
$(document).on('hidden.bs.modal', stop);
$(document).on('hidden.bs.modal', assign_zero_round);
$(document).on('hidden.bs.modal', function () {
 location.reload();
});



function display_score_table(){
  database.once('value', function(snapshot){
    if(snapshot.exists()){
      var content = '';
      snapshot.forEach(function(data){
        var val = data.val();
        content += '<tr>';
        content += '<td>' + val.username + '</td>';
        content += '<td>' + val.time + '</td>';
        content += '</tr>';
      });
      $('#tbody_ele').append(content);
    }
  });
}

function sort_score(){
  var all_data = [];
  database.once('value', function(snapshot){
    if(snapshot.exists()){
      snapshot.forEach(function(data){
        var val = data.val();
        var temp_arr = [val.time, val.username];
        all_data.push(temp_arr);
      });      
      all_data = all_data.sort(Comparator);
      all_data.forEach(sort_array);
    }
  });
}

function sort_array(item, index){
  var display_time = "";
  var current_time = item[0];
  var min = Math.floor(current_time / 60000);
    var sec = Math.floor(current_time / 1000);
    if(min < 1 && sec < 1){
      display_time = "0 min 0 sec." + current_time;
    }
    if(sec >= 1 && min < 1){
      display_time = "0 min " +  (Math.floor((current_time - 60000 * min) / 1000)).toString() + " sec." + (current_time - 1000 * sec).toString();
    }
    if(min >= 1){
      display_time = min + " min " +  (Math.floor((current_time - 60000 * min) / 1000)).toString() + " sec." + (current_time - 1000 * sec).toString();
    }
  var content = '';
  content += '<tr>';
  content += '<td>' + (index + 1).toString() + '</td>';
  content += '<td>' + item[1] + '</td>';
  content += '<td>' + display_time + '</td>';
  content += '</tr>';
  $('#tbody_ele').append(content);

}

function Comparator(a, b) {
  return a[0] - b[0];
 }

//display_score_table();
sort_score();



