select count(distinct p.type)
from CatchedPokemon c, Pokemon p
where c.pid = p.id and
      c.owner_id in (select leader_id
          from Gym
          where city = 'Sangnok City');