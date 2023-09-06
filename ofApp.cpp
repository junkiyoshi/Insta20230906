#include "ofApp.h"	

//--------------------------------------------------------------
Actor::Actor(vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	this->select_index = ofRandom(location_list.size());
	while (true) {

		auto itr = find(destination_list.begin(), destination_list.end(), this->select_index);
		if (itr == destination_list.end()) {

			destination_list.push_back(this->select_index);
			break;
		}

		this->select_index = (this->select_index + 1) % location_list.size();
	}

	this->next_index = this->select_index;
}

//--------------------------------------------------------------
void Actor::update(const int& frame_span, vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	if (ofGetFrameNum() % frame_span == 0) {

		auto tmp_index = this->select_index;
		this->select_index = this->next_index;
		int retry = next_index_list[this->select_index].size();
		this->next_index = next_index_list[this->select_index][(int)ofRandom(next_index_list[this->select_index].size())];
		while (--retry > 0) {

			auto destination_itr = find(destination_list.begin(), destination_list.end(), this->next_index);
			if (destination_itr == destination_list.end()) {

				if (tmp_index != this->next_index) {

					destination_list.push_back(this->next_index);
					break;
				}
			}

			this->next_index = next_index_list[this->select_index][(this->next_index + 1) % next_index_list[this->select_index].size()];
		}
		if (retry <= 0) {

			destination_list.push_back(this->select_index);
			this->next_index = this->select_index;
		}
	}

	auto param = ofGetFrameNum() % frame_span;
	auto distance = location_list[this->next_index] - location_list[this->select_index];
	this->location = location_list[this->select_index] + distance / frame_span * param;

	this->log.push_back(this->location);
	while (this->log.size() > 20) { this->log.erase(this->log.begin()); }
}

//--------------------------------------------------------------
glm::vec3 Actor::getLocation() {

	return this->location;
}

//--------------------------------------------------------------
glm::vec3 Actor::getLocation(int i) {

	return i > 0 && i < this->log.size() ? this->log[i] : glm::vec3();
}

//--------------------------------------------------------------
vector<glm::vec3> Actor::getLog() {

	return this->log;
}

//--------------------------------------------------------------
void Actor::setColor(ofColor value) {

	this->color = value;
}

//--------------------------------------------------------------
ofColor Actor::getColor() {

	return this->color;
}


//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(25);
	ofSetWindowTitle("openFrameworks");

	ofBackground(239);
	ofSetLineWidth(2);
	ofEnableBlendMode(ofBlendMode::OF_BLENDMODE_MULTIPLY);

	ofColor color;
	vector<ofColor> base_color_list;
	vector<int> hex_list = { 0xee6352, 0x59cd90, 0x3fa7d6, 0xfac05e, 0xf79d84 };
	for (auto hex : hex_list) {

		color.setHex(hex);
		base_color_list.push_back(color);
	}

	int span = 60;
	for (int x = -span * 5; x <= span * 5; x += span) {

		for (int y = -span * 5; y <= span * 5; y += span) {

			this->parent_location_group.push_back(glm::vec3(x, y, 0));
		}
	}

	for (auto& location : this->parent_location_group) {

		vector<int> next_index = vector<int>();
		int index = -1;
		for (auto& other : this->parent_location_group) {

			index++;
			if (location == other) { continue; }

			float distance = glm::distance(location, other);
			if (distance <= span) {

				next_index.push_back(index);
			}
		}
		this->parent_next_index_group.push_back(next_index);
	}

	for (int i = 0; i < base_color_list.size() * 3; i++) {

		this->parent_actor_group.push_back(make_unique<Actor>(this->parent_location_group, this->parent_next_index_group, this->parent_destination_group));
		this->parent_actor_group.back()->setColor(base_color_list[i % base_color_list.size()]);
	}

	span = 25;
	for (int g = 0; g < this->parent_actor_group.size(); g++) {

		vector<glm::vec3> location_group;
		for (int x = -span ; x <= span ; x += span) {

			for (int y = -span ; y <= span; y += span) {

				location_group.push_back(glm::vec3(x, y, 0));
			}
		}

		location_group_list.push_back(location_group);

		vector<vector<int>> next_index_group;
		for (auto& location : location_group) {

			vector<int> next_index = vector<int>();
			int index = -1;
			for (auto& other : location_group) {

				index++;
				if (location == other) { continue; }

				float distance = glm::distance(location, other);
				if (distance <= span) {

					next_index.push_back(index);
				}
			}

			next_index_group.push_back(next_index);
		}
		this->next_index_group_list.push_back(next_index_group);


		vector<unique_ptr<Actor>> actor_group;
		vector<int> destination_group;
		for (int i = 0; i < 6; i++) {

			auto a = make_unique<Actor>(location_group, next_index_group, destination_group);
			actor_group.push_back(move(a));
			actor_group.back()->setColor(this->parent_actor_group[g]->getColor());
		}

		this->actor_group_list.push_back(move(actor_group));
		this->destination_group_list.push_back(destination_group);
	}
}

//--------------------------------------------------------------
void ofApp::update() {

	int frame_span = 4;
	int prev_index_size = 0;

	if (ofGetFrameNum() % frame_span == 0) {

		prev_index_size = this->parent_destination_group.size();
	}

	for (auto& actor : this->parent_actor_group) {

		actor->update(frame_span, this->parent_location_group, this->parent_next_index_group, this->parent_destination_group);
	}

	if (prev_index_size != 0) {

		this->parent_destination_group.erase(this->parent_destination_group.begin(), this->parent_destination_group.begin() + prev_index_size);
	}

	frame_span = 2;
	for (int g = 0; g < this->parent_actor_group.size(); g++) {

		if (ofGetFrameNum() % frame_span == 0) {

			prev_index_size = this->destination_group_list[g].size();
		}

		for (auto& actor : this->actor_group_list[g]) {

			actor->update(frame_span, this->location_group_list[g], this->next_index_group_list[g], this->destination_group_list[g]);
		}

		if (prev_index_size != 0) {

			this->destination_group_list[g].erase(this->destination_group_list[g].begin(), this->destination_group_list[g].begin() + prev_index_size);
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	ofTranslate(ofGetWindowSize() * 0.5);

	for (int g = 0; g < parent_actor_group.size(); g++) {

		for (auto& actor : this->actor_group_list[g]) {

			ofSetColor(actor->getColor());

			ofNoFill();
			ofBeginShape();
			for (int i = 0; i < actor->getLog().size(); i++) {

				ofVertex(actor->getLog()[i] + this->parent_actor_group[g]->getLog()[i]);
				
			}
			ofEndShape();

			ofFill();
			ofDrawCircle(actor->getLocation() + this->parent_actor_group[g]->getLocation(), 5);
		}
	}
}


//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}